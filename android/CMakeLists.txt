cmake_minimum_required(VERSION 3.4.1)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX11_EXTENSION_COMPILE_OPTION "-std=c++17")

find_library(log-lib log)

add_library(cpp
            SHARED
            ../cpp/react-native-webassembly.cpp
            cpp-adapter.cpp
            # wasm3
            ../cpp/m3_api_libc.c
            ../cpp/m3_api_libc.h
            ../cpp/m3_api_meta_wasi.c
            ../cpp/m3_api_tracer.c
            ../cpp/m3_api_tracer.h
            ../cpp/m3_api_uvwasi.c
            ../cpp/m3_api_wasi.c
            ../cpp/m3_api_wasi.h
            ../cpp/m3_bind.c
            ../cpp/m3_bind.h
            ../cpp/m3_code.c
            ../cpp/m3_code.h
            ../cpp/m3_compile.c
            ../cpp/m3_compile.h
            ../cpp/m3_config.h
            ../cpp/m3_config_platforms.h
            ../cpp/m3_core.c
            ../cpp/m3_core.h
            ../cpp/m3_env.c
            ../cpp/m3_env.h
            ../cpp/m3_exception.h
            ../cpp/m3_exec.c
            ../cpp/m3_exec.h
            ../cpp/m3_exec_defs.h
            ../cpp/m3_function.c
            ../cpp/m3_function.h
            ../cpp/m3_info.c
            ../cpp/m3_info.h
            ../cpp/m3_math_utils.h
            ../cpp/m3_module.c
            ../cpp/m3_parse.c
            ../cpp/react-native-webassembly.h
            ../cpp/wasm3.h
            ../cpp/wasm3_cpp.h
            ../cpp/wasm3_defs.h
        )

# Specifies a path to native header files.
include_directories(
            ../cpp
)

find_package(ReactAndroid REQUIRED CONFIG)

target_link_libraries(
        cpp
        ${log-lib}
        ReactAndroid::jsi
        android
)
