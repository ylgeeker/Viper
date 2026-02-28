project(vipercore)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
set(CMAKE_SHARED_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})

message("libcore Output Dir: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX "test")

# ==================== Dependency Discovery ====================

# yaml-cpp (static linking)
if(YamlCpp_FOUND)
    set(yaml-cpp_DIR ${YAML_CPP_PREFIX}/lib/cmake/yaml-cpp)
endif()
find_package(yaml-cpp REQUIRED)

# spdlog (static linking)
if(Spdlog_FOUND)
    set(spdlog_DIR ${SPDLOG_PREFIX}/lib/cmake/spdlog)
endif()
find_package(spdlog REQUIRED)

# CURL (static linking)
if(CURL_FOUND)
    set(CURL_INCLUDE_DIR ${CURL_PREFIX}/include)
    set(CURL_LIBRARY ${CURL_PREFIX}/lib/libcurl.a)
    add_library(CURL::libcurl STATIC IMPORTED)
    set_target_properties(CURL::libcurl PROPERTIES
        IMPORTED_LOCATION ${CURL_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${CURL_INCLUDE_DIR}
    )
else()
    find_package(CURL REQUIRED)
endif()

# libevent (static linking)
set(LIBEVENT_INCLUDE_DIR ${LIBEVENT_PREFIX}/include)
set(LIBEVENT_LIBRARY ${LIBEVENT_PREFIX}/lib/libevent.a)
set(LIBEVENT_CORE_LIBRARY ${LIBEVENT_PREFIX}/lib/libevent_core.a)
set(LIBEVENT_EXTRA_LIBRARY ${LIBEVENT_PREFIX}/lib/libevent_extra.a)
set(LIBEVENT_PTHREADS_LIBRARY ${LIBEVENT_PREFIX}/lib/libevent_pthreads.a)

if(EXISTS ${LIBEVENT_CORE_LIBRARY})
    add_library(Libevent::core STATIC IMPORTED)
    set_target_properties(Libevent::core PROPERTIES
        IMPORTED_LOCATION ${LIBEVENT_CORE_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${LIBEVENT_INCLUDE_DIR}
    )
    set(LIBEVENT_FOUND TRUE)
endif()

# etcd-cpp-apiv3 (static linking). Default build installs only libetcd-cpp-api.a;
# etcd-cpp-api-core is INTERFACE (no .a). Accept single .a so controller can link.
set(ETCD_CPP_INCLUDE_DIR ${ETCD_CPP_PREFIX}/include)
set(ETCD_CPP_LIBRARY ${ETCD_CPP_PREFIX}/lib/libetcd-cpp-api.a)
set(ETCD_CPP_CORE_LIBRARY ${ETCD_CPP_PREFIX}/lib/libetcd-cpp-api-core.a)

set(ETCD_CPP_FOUND FALSE)
if(EXISTS ${ETCD_CPP_LIBRARY})
    add_library(etcd-cpp-api STATIC IMPORTED)
    set_target_properties(etcd-cpp-api PROPERTIES
        IMPORTED_LOCATION ${ETCD_CPP_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${ETCD_CPP_INCLUDE_DIR}
    )
    if(EXISTS ${ETCD_CPP_CORE_LIBRARY})
        add_library(etcd-cpp-api-core STATIC IMPORTED)
        set_target_properties(etcd-cpp-api-core PROPERTIES
            IMPORTED_LOCATION ${ETCD_CPP_CORE_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${ETCD_CPP_INCLUDE_DIR}
        )
    else()
        add_library(etcd-cpp-api-core INTERFACE)
        target_link_libraries(etcd-cpp-api-core INTERFACE etcd-cpp-api)
        target_include_directories(etcd-cpp-api-core INTERFACE ${ETCD_CPP_INCLUDE_DIR})
    endif()
    set(ETCD_CPP_FOUND TRUE)
endif()
set(ETCD_CPP_FOUND ${ETCD_CPP_FOUND} PARENT_SCOPE)
if(NOT ETCD_CPP_FOUND)
    list(REMOVE_ITEM SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/app/service_registry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/service_discovery.cpp
    )
endif()

# concurrentqueue (header-only)
set(CONCURRENTQUEUE_INCLUDE_DIR ${CONCURRENTQUEUE_PREFIX}/include)

# RapidJSON (header-only)
set(RAPIDJSON_INCLUDE_DIR ${RAPIDJSON_PREFIX}/include)

# ==================== Build Library ====================

add_library(${PROJECT_NAME} STATIC ${SOURCES})

# If external dependencies need to be installed first, add dependency relationships
if(HAS_EXTERNAL_DEPS)
    add_dependencies(${PROJECT_NAME} install_dependencies)
endif()

# yaml-cpp and spdlog PUBLIC: core's public headers need them
target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp::yaml-cpp spdlog::spdlog)

# CURL PRIVATE: only used in implementation
if(TARGET CURL::libcurl)
    target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
endif()

# libevent PUBLIC: core's public headers include event2/*
if(EXISTS ${LIBEVENT_INCLUDE_DIR})
    target_include_directories(${PROJECT_NAME} PUBLIC ${LIBEVENT_INCLUDE_DIR})
endif()

if(TARGET Libevent::core)
    target_link_libraries(${PROJECT_NAME} PUBLIC Libevent::core)
    if(EXISTS ${LIBEVENT_EXTRA_LIBRARY})
        target_link_libraries(${PROJECT_NAME} PRIVATE 
            ${LIBEVENT_EXTRA_LIBRARY}
            ${LIBEVENT_PTHREADS_LIBRARY}
        )
    endif()
endif()

# etcd PUBLIC: core's public headers include etcd/Client.hpp
if(ETCD_CPP_FOUND AND TARGET etcd-cpp-api)
    target_include_directories(${PROJECT_NAME} PUBLIC ${ETCD_CPP_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PUBLIC etcd-cpp-api etcd-cpp-api-core)
else()
    # Fallback: just add include directory if library exists
    if(EXISTS ${ETCD_CPP_INCLUDE_DIR})
        target_include_directories(${PROJECT_NAME} PUBLIC ${ETCD_CPP_INCLUDE_DIR})
    endif()
endif()

# concurrentqueue PUBLIC: core's public headers include it
if(EXISTS ${CONCURRENTQUEUE_INCLUDE_DIR})
    target_include_directories(${PROJECT_NAME} PUBLIC ${CONCURRENTQUEUE_INCLUDE_DIR})
endif()

# RapidJSON PUBLIC: core's public headers include it
if(RAPIDJSON_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE_DIR})
endif()

# OpenSSL (static linking)
if(OPENSSL_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ${OPENSSL_PREFIX}/lib64/libssl.a
        ${OPENSSL_PREFIX}/lib64/libcrypto.a
    )
    target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_PREFIX}/include)
endif()

# System libraries
target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)

# Optional: build shared library
option(BUILD_CORE_SHARED "Build libcore_shared.so" OFF)
if(BUILD_CORE_SHARED)
    add_library(${PROJECT_NAME}_shared SHARED ${SOURCES})
    target_link_libraries(${PROJECT_NAME}_shared PRIVATE yaml-cpp::yaml-cpp spdlog::spdlog)
    if(HAS_EXTERNAL_DEPS)
        add_dependencies(${PROJECT_NAME}_shared install_dependencies)
    endif()
endif()
