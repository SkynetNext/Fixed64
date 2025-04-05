cmake_minimum_required(VERSION 3.10)
project(Fixed64 VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(Fixed64 INTERFACE)
target_include_directories(Fixed64 INTERFACE ${PROJECT_SOURCE_DIR}/include)

# Enable testing
enable_testing()

# Add CTest support, this will generate DartConfiguration.tcl
include(CTest)
set(CTEST_PROJECT_NAME "Fixed64")
set(CTEST_NIGHTLY_START_TIME "00:00:00 UTC")

set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib" FORCE)

# Add Google Test
add_subdirectory(third_party/googletest)

# Configure CTest to show output from all tests
set(CMAKE_CTEST_ARGUMENTS "--output-on-failure" "--verbose")

# Add tests directory
add_subdirectory(tests)

# Create example programs (if example code exists)
if(EXISTS "${PROJECT_SOURCE_DIR}/examples")
    file(GLOB EXAMPLE_SOURCES "examples/*.cpp")
    foreach(example_source ${EXAMPLE_SOURCES})
        get_filename_component(example_name ${example_source} NAME_WE)
        add_executable(${example_name} ${example_source})
        target_link_libraries(${example_name} Fixed64)
        
        # Set compiler-specific warning flags
        if(MSVC)
            # MSVC compiler flags
            set(COMPILER_WARNINGS
                /W4
                /WX-      # Don't treat warnings as errors
                /wd4127   # Disable warning C4127: conditional expression is constant
                /wd4244   # Disable warning C4244: conversion from 'int64_t' to 'int', possible loss of data
                /wd4146   # Disable warning C4146: unary minus operator applied to unsigned type
                /wd4996   # Disable specific warnings, e.g., deprecated function warnings
            )
        else()
            # GCC/Clang compiler flags
            set(COMPILER_WARNINGS
                -Wall
                -Wextra
                -O3
            )
        endif()
        
        # Apply conditional compiler flags
        target_compile_options(${example_name} PRIVATE ${COMPILER_WARNINGS})
    endforeach()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add Berkeley SoftFloat support for benchmarking
option(ENABLE_SOFTFLOAT_BENCHMARK "Enable benchmarking against Berkeley SoftFloat" ON)

if(ENABLE_SOFTFLOAT_BENCHMARK)
    # Define SoftFloat paths with absolute paths
    set(SOFTFLOAT_DIR ${PROJECT_SOURCE_DIR}/third_party/berkeley-softfloat-3)
    
    # Detect platform and use appropriate pre-compiled library
    if(WIN32)
        set(SOFTFLOAT_LIBRARY "${CMAKE_SOURCE_DIR}/prebuilt/softfloat/windows/softfloat.a")
    elseif(APPLE)
        set(SOFTFLOAT_LIBRARY "${CMAKE_SOURCE_DIR}/prebuilt/softfloat/macos/softfloat.a")
    elseif(UNIX)
        set(SOFTFLOAT_LIBRARY "${CMAKE_SOURCE_DIR}/prebuilt/softfloat/linux/softfloat.a")
    endif()

    # Only if prebuilt library doesn't exist, use the submodule for building
    if(NOT EXISTS ${SOFTFLOAT_LIBRARY})
        message(STATUS "Pre-compiled SoftFloat not found, will build from source")
        # Your existing SoftFloat build logic here
    endif()
    
    # Create imported library target
    add_library(softfloat STATIC IMPORTED GLOBAL)
    set_target_properties(softfloat PROPERTIES
        IMPORTED_LOCATION "${SOFTFLOAT_LIBRARY}"
    )
    
    # Set include directories
    set(SOFTFLOAT_INCLUDES 
        ${SOFTFLOAT_DIR}/source/include
        ${SOFTFLOAT_DIR}/source/RISCV
    )
    
    # Add benchmark directory
    add_subdirectory(benchmarks)
endif()
