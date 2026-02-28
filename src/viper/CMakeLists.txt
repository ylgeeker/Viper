project(viper)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})

message("Viper Output Dir: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../internal/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../internal/*.cc")

list(FILTER SOURCES EXCLUDE REGEX "test")

add_executable(${PROJECT_NAME} ${SOURCES})

find_file(Z_STATIC_A NAMES libz.a PATHS /usr/lib/x86_64-linux-gnu /usr/lib NO_DEFAULT_PATH)

# Link OpenSSL static libs with --whole-archive when using static deps (same strategy as controller)
if(CONTROLLER_USE_STATIC_DEPS AND TARGET viper_static_deps)
    if(EXISTS "${OPENSSL_PREFIX}/lib64/libssl.a")
        target_link_libraries(${PROJECT_NAME} PRIVATE
            -Wl,--whole-archive
            ${OPENSSL_PREFIX}/lib64/libssl.a
            ${OPENSSL_PREFIX}/lib64/libcrypto.a
            -Wl,--no-whole-archive)
    elseif(EXISTS "${OPENSSL_PREFIX}/lib/libssl.a")
        target_link_libraries(${PROJECT_NAME} PRIVATE
            -Wl,--whole-archive
            ${OPENSSL_PREFIX}/lib/libssl.a
            ${OPENSSL_PREFIX}/lib/libcrypto.a
            -Wl,--no-whole-archive)
    endif()
endif()

find_package(ZLIB REQUIRED)
if(ABSL_USE_STATIC_TARGET)
    # Use static Abseil (absl_static) from top-level CMakeLists
else()
    find_package(absl REQUIRED COMPONENTS base log)
endif()
find_package(protobuf REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROTOBUF_PREFIX}/include)
# Override protobuf to static .a when available (same as controller)
if(CONTROLLER_STATIC_HAS_PROTOBUF AND TARGET protobuf::libprotobuf AND EXISTS "${PROTOBUF_PREFIX}/lib/libprotobuf.a")
    set_target_properties(protobuf::libprotobuf PROPERTIES
        IMPORTED_LOCATION "${PROTOBUF_PREFIX}/lib/libprotobuf.a"
        IMPORTED_LOCATION_RELEASE "${PROTOBUF_PREFIX}/lib/libprotobuf.a")
endif()

list(APPEND LIBS core)
list(APPEND LIBS yaml-cpp)
list(APPEND LIBS pthread)
list(APPEND LIBS stdc++fs)
# Unified linking: prefer viper_static_deps, fallback to dynamic (same strategy as controller)
if(CONTROLLER_USE_STATIC_DEPS AND TARGET viper_static_deps)
    list(APPEND LIBS viper_static_deps)
    # protobuf::libprotobuf added once below (IMPORTED_LOCATION overridden to our .a when CONTROLLER_STATIC_HAS_PROTOBUF)
    if(NOT CONTROLLER_STATIC_HAS_CPPREST)
        list(APPEND LIBS cpprest)
    endif()
else()
    list(APPEND LIBS ZLIB::ZLIB)
    list(APPEND LIBS curl)
    list(APPEND LIBS event)
    list(APPEND LIBS event_openssl)
    list(APPEND LIBS cpprest)
    list(APPEND LIBS protobuf::libprotobuf)
    list(APPEND LIBS ZLIB::ZLIB)
endif()
if(ABSL_USE_STATIC_TARGET)
    list(APPEND LIBS absl_static)
else()
    list(APPEND LIBS absl::base)
    list(APPEND LIBS absl::log)
endif()
# Always link protobuf via target (IMPORTED_LOCATION set to our .a when CONTROLLER_STATIC_HAS_PROTOBUF); not in viper_static_deps to avoid duplicate symbols.
list(APPEND LIBS protobuf::libprotobuf)

add_library(viper_main_group INTERFACE)
target_link_libraries(viper_main_group INTERFACE -Wl,--start-group ${LIBS} -Wl,--end-group)
target_link_libraries(${PROJECT_NAME} PRIVATE viper_main_group)

# z/zstd handling (same as controller)
if(CONTROLLER_USE_STATIC_DEPS)
    target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--no-as-needed)
    if(EXISTS "/usr/lib/x86_64-linux-gnu/libz.a")
        target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/x86_64-linux-gnu/libz.a)
    elseif(EXISTS "/usr/lib/libz.a")
        target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/libz.a)
    elseif(EXISTS "/usr/lib/x86_64-linux-gnu/libz.so.1")
        target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/x86_64-linux-gnu/libz.so.1)
    elseif(EXISTS "/lib/x86_64-linux-gnu/libz.so.1")
        target_link_libraries(${PROJECT_NAME} PRIVATE /lib/x86_64-linux-gnu/libz.so.1)
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE -lz)
    endif()
endif()
if(CONTROLLER_USE_STATIC_DEPS AND NOT CONTROLLER_STATIC_HAS_ZSTD)
    if(EXISTS "/usr/lib/x86_64-linux-gnu/libzstd.so.1")
        target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/x86_64-linux-gnu/libzstd.so.1)
    elseif(EXISTS "/lib/x86_64-linux-gnu/libzstd.so.1")
        target_link_libraries(${PROJECT_NAME} PRIVATE /lib/x86_64-linux-gnu/libzstd.so.1)
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE -lzstd)
    endif()
endif()
if(CONTROLLER_USE_STATIC_DEPS)
    target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--as-needed)
endif()
if(NOT (CONTROLLER_USE_STATIC_DEPS AND TARGET viper_static_deps))
    target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--no-as-needed)
    if(EXISTS "/usr/lib/x86_64-linux-gnu/libz.so.1")
        target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/x86_64-linux-gnu/libz.so.1)
    elseif(EXISTS "/lib/x86_64-linux-gnu/libz.so.1")
        target_link_libraries(${PROJECT_NAME} PRIVATE /lib/x86_64-linux-gnu/libz.so.1)
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE -lz)
    endif()
    if(EXISTS "/usr/lib/x86_64-linux-gnu/libzstd.so.1")
        target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/x86_64-linux-gnu/libzstd.so.1)
    elseif(EXISTS "/lib/x86_64-linux-gnu/libzstd.so.1")
        target_link_libraries(${PROJECT_NAME} PRIVATE /lib/x86_64-linux-gnu/libzstd.so.1)
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--as-needed)
endif()
if(Z_STATIC_A)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${Z_STATIC_A})
endif()
