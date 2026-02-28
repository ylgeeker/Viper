cmake_minimum_required(VERSION 3.10)

project(Viper VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(OUTPUT_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(OUTPUT_LIB_DIR ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Set CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Set parallel compilation count
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N} CACHE STRING "Parallel build jobs")
endif()

option(BUILD_TESTS "Build test cases" OFF)
option(AUTO_INSTALL_DEPENDENCIES "Automatically install missing dependencies" ON)

string(TIMESTAMP BUILT_TIME "%Y-%m-%d %H:%M:%S")

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT DEFINED VERSION)
    set(VERSION "unknown")
endif()

add_definitions(-DRAPIDJSON_HAS_STDSTRING -DRAPIDJSON_NOMEMBERITERATORCLASS)

# Project source include path
include_directories(${CMAKE_SOURCE_DIR}/src)

# When both static and shared libraries exist, prefer static (.a before .so)
set(CMAKE_FIND_PREFER_STATIC_LIBRARY TRUE CACHE BOOL "Prefer static libs when both .a and .so exist")

# Load dependency management system
include(Dependencies)

# Prefer our OpenSSL static libs so gRPC/etcd use them (and libz.a) instead of system libssl/libcrypto
if(OpenSSL_FOUND AND OPENSSL_PREFIX)
    set(OPENSSL_ROOT_DIR ${OPENSSL_PREFIX} CACHE PATH "OpenSSL root for find_package(OpenSSL)" FORCE)
    set(OPENSSL_USE_STATIC_LIBS TRUE CACHE BOOL "Prefer OpenSSL static libs")
    if(EXISTS "${OPENSSL_PREFIX}/lib64/libssl.a")
        set(OPENSSL_SSL_LIBRARY "${OPENSSL_PREFIX}/lib64/libssl.a" CACHE FILEPATH "OpenSSL SSL library" FORCE)
        set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_PREFIX}/lib64/libcrypto.a" CACHE FILEPATH "OpenSSL Crypto library" FORCE)
        # Create OpenSSL targets with .a so find_package(OpenSSL) in gRPC does not overwrite with .so
        if(NOT TARGET OpenSSL::Crypto)
            add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
            set_target_properties(OpenSSL::Crypto PROPERTIES
                IMPORTED_LOCATION "${OPENSSL_PREFIX}/lib64/libcrypto.a"
                INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_PREFIX}/include")
        endif()
        if(NOT TARGET OpenSSL::SSL)
            add_library(OpenSSL::SSL UNKNOWN IMPORTED)
            set_target_properties(OpenSSL::SSL PROPERTIES
                IMPORTED_LOCATION "${OPENSSL_PREFIX}/lib64/libssl.a"
                INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_PREFIX}/include"
                INTERFACE_LINK_LIBRARIES OpenSSL::Crypto)
        endif()
    elseif(EXISTS "${OPENSSL_PREFIX}/lib/libssl.a")
        set(OPENSSL_SSL_LIBRARY "${OPENSSL_PREFIX}/lib/libssl.a" CACHE FILEPATH "OpenSSL SSL library" FORCE)
        set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_PREFIX}/lib/libcrypto.a" CACHE FILEPATH "OpenSSL Crypto library" FORCE)
        if(NOT TARGET OpenSSL::Crypto)
            add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
            set_target_properties(OpenSSL::Crypto PROPERTIES
                IMPORTED_LOCATION "${OPENSSL_PREFIX}/lib/libcrypto.a"
                INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_PREFIX}/include")
        endif()
        if(NOT TARGET OpenSSL::SSL)
            add_library(OpenSSL::SSL UNKNOWN IMPORTED)
            set_target_properties(OpenSSL::SSL PROPERTIES
                IMPORTED_LOCATION "${OPENSSL_PREFIX}/lib/libssl.a"
                INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_PREFIX}/include"
                INTERFACE_LINK_LIBRARIES OpenSSL::Crypto)
        endif()
    endif()
endif()

# Set utf8_range path for Protobuf (if Protobuf is found)
if(Protobuf_FOUND OR EXISTS "${PROTOBUF_PREFIX}/lib/cmake/utf8_range")
    set(utf8_range_DIR "${PROTOBUF_PREFIX}/lib/cmake/utf8_range" CACHE PATH "utf8_range config dir")
endif()

# Force Protobuf_DIR so find_package(protobuf) in subdirs uses our install (required for runtime_version.h)
if(Protobuf_FOUND OR EXISTS "${PROTOBUF_PREFIX}/lib/cmake/protobuf")
    set(Protobuf_DIR "${PROTOBUF_PREFIX}/lib/cmake/protobuf" CACHE PATH "Protobuf config dir" FORCE)
endif()

# Set absl_DIR if found (controller overrides Abseil targets to static .a after find_package(gRPC))
if(absl_FOUND OR EXISTS "${ABSEIL_PREFIX}/lib/cmake/absl")
    set(absl_DIR "${ABSEIL_PREFIX}/lib/cmake/absl" CACHE PATH "Abseil config dir")
endif()

# Prefer static Abseil to avoid runtime dependency on libabsl_*.so
if(Abseil_FOUND AND EXISTS "${ABSEIL_PREFIX}/lib/libabsl_base.a")
    file(GLOB ABSL_STATIC_LIBS "${ABSEIL_PREFIX}/lib/libabsl_*.a")
    add_library(absl_static INTERFACE)
    target_include_directories(absl_static INTERFACE ${ABSEIL_PREFIX}/include)
    target_link_libraries(absl_static INTERFACE -Wl,--start-group ${ABSL_STATIC_LIBS} -Wl,--end-group)
    set(ABSL_USE_STATIC_TARGET TRUE CACHE BOOL "Use static Abseil target")
endif()

# Set gRPC_DIR if found
if(gRPC_FOUND OR EXISTS "${GRPC_PREFIX}/lib/cmake/grpc")
    set(gRPC_DIR "${GRPC_PREFIX}/lib/cmake/grpc" CACHE PATH "gRPC config dir")
endif()

# Static deps for controller/viper/transfer: one interface target linking all .a + -static-libstdc++
# so binaries do not depend on libcurl.so, libevent.so, libprotobuf.so, libz.so, libzstd.so, libstdc++.so, libabsl_*.so
# Abstract as COMMON_STATIC_DEPS_LIBS / viper_static_deps for viper/transfer/controller to share (prefer static, allow dynamic fallback)
set(COMMON_STATIC_DEPS_LIBS "")
if(EXISTS "${CURL_PREFIX}/lib/libcurl.a")
    list(APPEND COMMON_STATIC_DEPS_LIBS ${CURL_PREFIX}/lib/libcurl.a)
endif()
if(EXISTS "${LIBEVENT_PREFIX}/lib/libevent_core.a")
    list(APPEND COMMON_STATIC_DEPS_LIBS
        ${LIBEVENT_PREFIX}/lib/libevent_core.a
        ${LIBEVENT_PREFIX}/lib/libevent_extra.a
        ${LIBEVENT_PREFIX}/lib/libevent_pthreads.a
        ${LIBEVENT_PREFIX}/lib/libevent_openssl.a)
endif()
# Do not add libprotobuf.a to COMMON_STATIC_DEPS_LIBS: viper/transfer/controller link via protobuf::libprotobuf
# (with IMPORTED_LOCATION set to our .a when available) to avoid duplicate symbols and ensure one protobuf.
find_file(PROTOBUF_STATIC_A NAMES libprotobuf.a PATHS ${PROTOBUF_PREFIX}/lib /usr/lib/x86_64-linux-gnu /usr/lib NO_DEFAULT_PATH)
if(PROTOBUF_STATIC_A)
    if(EXISTS "${PROTOBUF_PREFIX}/lib/libutf8_validity.a")
        list(APPEND COMMON_STATIC_DEPS_LIBS ${PROTOBUF_PREFIX}/lib/libutf8_validity.a)
    endif()
    set(CONTROLLER_STATIC_HAS_PROTOBUF TRUE CACHE BOOL "")
endif()
# z and zstd: prefer .a from project deps or system
foreach(_zname z zstd)
    if(_zname STREQUAL "z")
        set(_paths "/usr/lib/x86_64-linux-gnu" "/usr/lib")
    else()
        set(_paths "${ZSTD_PREFIX}/lib" "/usr/lib/x86_64-linux-gnu" "/usr/lib" "${PROTOBUF_PREFIX}/lib")
    endif()
    find_file(_zlib_${_zname} NAMES lib${_zname}.a PATHS ${_paths} NO_DEFAULT_PATH)
    if(_zlib_${_zname})
        list(APPEND COMMON_STATIC_DEPS_LIBS ${_zlib_${_zname}})
        if(_zname STREQUAL "zstd")
            set(CONTROLLER_STATIC_HAS_ZSTD TRUE CACHE BOOL "")
        endif()
    endif()
endforeach()
# cpprest: static if available (prefer project install)
find_file(CPPREST_STATIC_A NAMES libcpprest.a PATHS ${CPPREST_PREFIX}/lib /usr/lib/x86_64-linux-gnu /usr/lib NO_DEFAULT_PATH)
if(CPPREST_STATIC_A)
    list(APPEND COMMON_STATIC_DEPS_LIBS ${CPPREST_STATIC_A})
    set(CONTROLLER_STATIC_HAS_CPPREST TRUE CACHE BOOL "")
endif()
# viper_static_deps: shared target for viper/transfer/controller (prefer static, allow dynamic fallback)
if(COMMON_STATIC_DEPS_LIBS)
    add_library(viper_static_deps INTERFACE)
    target_link_libraries(viper_static_deps INTERFACE -Wl,--start-group ${COMMON_STATIC_DEPS_LIBS} -Wl,--end-group -static-libstdc++)
    set(CONTROLLER_USE_STATIC_DEPS TRUE CACHE BOOL "Link viper/transfer/controller with static curl, libevent, protobuf, z, zstd, cpprest, stdc++")
    # Backward compat: controller_static_deps points to viper_static_deps
    add_library(controller_static_deps ALIAS viper_static_deps)
endif()

# Add library search paths for dependencies (so -lcurl, -lyaml-cpp, etc. resolve)
if(EXISTS "${CURL_PREFIX}/lib")
    link_directories(${CURL_PREFIX}/lib)
endif()
if(EXISTS "${YAML_CPP_PREFIX}/lib")
    link_directories(${YAML_CPP_PREFIX}/lib)
endif()
if(EXISTS "${LIBEVENT_PREFIX}/lib")
    link_directories(${LIBEVENT_PREFIX}/lib)
endif()
if(EXISTS "${ETCD_CPP_PREFIX}/lib")
    link_directories(${ETCD_CPP_PREFIX}/lib)
endif()
# Do not add OpenSSL link_directories: same dir has libssl.a and libssl.so.3; -L would let linker pick .so and cause "DSO missing from command line"
if(EXISTS "${CPPREST_PREFIX}/lib")
    link_directories(${CPPREST_PREFIX}/lib)
endif()
if(EXISTS "${ZSTD_PREFIX}/lib")
    link_directories(${ZSTD_PREFIX}/lib)
endif()

# install_dependencies target: always defined so "cmake --build . --target install_dependencies" never fails
add_custom_target(install_dependencies
    COMMENT "Installing external dependencies (no-op if all deps already found)..."
)
# If dependencies need to be installed, wire them to install_dependencies
set(HAS_EXTERNAL_DEPS FALSE)
if(OpenSSL_NEEDS_INSTALL OR Boost_NEEDS_INSTALL OR Libevent_NEEDS_INSTALL OR 
   CURL_NEEDS_INSTALL OR YamlCpp_NEEDS_INSTALL OR Spdlog_NEEDS_INSTALL OR
   RapidJSON_NEEDS_INSTALL OR Concurrentqueue_NEEDS_INSTALL OR
   Protobuf_NEEDS_INSTALL OR Abseil_NEEDS_INSTALL OR Cares_NEEDS_INSTALL OR
   Re2_NEEDS_INSTALL OR gRPC_NEEDS_INSTALL OR EtcdCpp_NEEDS_INSTALL OR
   Cpprest_NEEDS_INSTALL OR Zstd_NEEDS_INSTALL)
    set(HAS_EXTERNAL_DEPS TRUE)
    
    if(OpenSSL_NEEDS_INSTALL)
        add_dependencies(install_dependencies openssl_external)
    endif()
    if(Boost_NEEDS_INSTALL)
        add_dependencies(install_dependencies boost_external)
    endif()
    if(Libevent_NEEDS_INSTALL)
        add_dependencies(install_dependencies libevent_external)
    endif()
    if(CURL_NEEDS_INSTALL)
        add_dependencies(install_dependencies curl_external)
    endif()
    if(YamlCpp_NEEDS_INSTALL)
        add_dependencies(install_dependencies yaml-cpp_external)
    endif()
    if(Spdlog_NEEDS_INSTALL)
        add_dependencies(install_dependencies spdlog_external)
    endif()
    if(RapidJSON_NEEDS_INSTALL)
        add_dependencies(install_dependencies rapidjson_external)
    endif()
    if(Concurrentqueue_NEEDS_INSTALL)
        add_dependencies(install_dependencies concurrentqueue_external)
    endif()
    if(Abseil_NEEDS_INSTALL)
        add_dependencies(install_dependencies abseil_external)
    endif()
    if(Protobuf_NEEDS_INSTALL)
        add_dependencies(install_dependencies protobuf_external)
    endif()
    if(Cares_NEEDS_INSTALL)
        add_dependencies(install_dependencies cares_external)
    endif()
    if(Re2_NEEDS_INSTALL)
        add_dependencies(install_dependencies re2_external)
    endif()
    if(gRPC_NEEDS_INSTALL)
        add_dependencies(install_dependencies grpc_external)
    endif()
    if(EtcdCpp_NEEDS_INSTALL)
        add_dependencies(install_dependencies etcd-cpp_external)
    endif()
    if(Cpprest_NEEDS_INSTALL)
        add_dependencies(install_dependencies cpprest_external)
    endif()
    if(Zstd_NEEDS_INSTALL)
        add_dependencies(install_dependencies zstd_external)
    endif()
    
    message(STATUS "==========================================================")
    message(STATUS "Some dependencies need to be installed.")
    message(STATUS "Run: cmake --build . --target install_dependencies")
    message(STATUS "Then reconfigure: cmake ..")
    message(STATUS "==========================================================")
endif()
# install_dependencies always exists; when HAS_EXTERNAL_DEPS is false it has no deps (build is no-op)

# Only add core/viper/controller/transfer when deps are installed; otherwise find_package in core would fail
if(NOT HAS_EXTERNAL_DEPS)
    add_subdirectory(src/core)
    add_subdirectory(src/viper)
endif()
