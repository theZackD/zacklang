cmake_minimum_required(VERSION 3.20)

project(zack VERSION 0.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ZACK_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ZACK_SANITIZE "Build the compiler with AddressSanitizer and UndefinedBehaviorSanitizer (not for MSVC)" OFF)

# LLVM is pinned to major version 20 (decision D2). Upgrading is a planned task.
# LLVM's own version file only accepts an exact major.minor match, so find_package is called
# without a version and the major version is checked explicitly below. The hints cover the
# usual package locations; pass -DLLVM_DIR=<dir containing LLVMConfig.cmake> to override them.
set(ZACK_LLVM_MAJOR 20)
find_package(LLVM CONFIG
    HINTS
        /opt/homebrew/opt/llvm@${ZACK_LLVM_MAJOR}/lib/cmake/llvm
        /usr/local/opt/llvm@${ZACK_LLVM_MAJOR}/lib/cmake/llvm
        /usr/lib/llvm-${ZACK_LLVM_MAJOR}/lib/cmake/llvm
        /opt/homebrew/opt/llvm/lib/cmake/llvm)
if(NOT LLVM_FOUND)
    message(FATAL_ERROR
        "LLVM ${ZACK_LLVM_MAJOR} was not found. Install it and pass "
        "-DLLVM_DIR=<dir containing LLVMConfig.cmake>.")
endif()
if(NOT LLVM_VERSION_MAJOR EQUAL ZACK_LLVM_MAJOR)
    message(FATAL_ERROR
        "Found LLVM ${LLVM_PACKAGE_VERSION} in ${LLVM_DIR}, but ZackLang is pinned to "
        "LLVM ${ZACK_LLVM_MAJOR}. Pass -DLLVM_DIR=<LLVM ${ZACK_LLVM_MAJOR} cmake directory>.")
endif()
message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION} from ${LLVM_DIR}")

# Only the C API is used (decision D3). The components below cover building IR, running
# optimization passes and emitting object code for the host target.
llvm_map_components_to_libnames(ZACK_LLVM_LIBS
    core support analysis passes target native nativecodegen)

# Compiler warnings and sanitizers shared by all targets. Pass RUNTIME for code that is
# linked into user programs (libzackrt): it gets warnings but never sanitizers, because the
# programs the compiler produces are not linked against a sanitizer runtime.
function(zack_configure_target target)
    cmake_parse_arguments(ARG "RUNTIME" "" "" ${ARGN})
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
        if(ZACK_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
        if(ZACK_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
        if(ZACK_SANITIZE AND NOT ARG_RUNTIME)
            target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address,undefined)
        endif()
    endif()
endfunction()

# C runtime library linked into every compiled program (decision D46).
add_library(zackrt STATIC runtime/zackrt.c)
target_include_directories(zackrt PUBLIC runtime)
zack_configure_target(zackrt RUNTIME)

# The only library that includes llvm-c headers (decision D3).
add_library(zack_codegen STATIC src/codegen/backend.cpp)
target_include_directories(zack_codegen PUBLIC src)
target_include_directories(zack_codegen SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(zack_codegen PRIVATE ${ZACK_LLVM_LIBS})
zack_configure_target(zack_codegen)

# Process launching and linking; used by the driver, the spike tool and the test runner.
add_library(zack_driver STATIC src/driver/process.cpp src/driver/linker.cpp)
target_include_directories(zack_driver PUBLIC src)
find_package(Threads REQUIRED)
target_link_libraries(zack_driver PUBLIC Threads::Threads)
zack_configure_target(zack_driver)

add_executable(zackc src/driver/main.cpp)
target_link_libraries(zackc PRIVATE zack_codegen zack_driver)
target_compile_definitions(zackc PRIVATE ZACK_VERSION="${PROJECT_VERSION}")
zack_configure_target(zackc)

# Phase 0 spike: builds tiny programs through the Backend to prove the toolchain.
add_executable(backend_spike tools/backend_spike.cpp)
target_link_libraries(backend_spike PRIVATE zack_codegen zack_driver)
zack_configure_target(backend_spike)

enable_testing()
add_subdirectory(tests)
