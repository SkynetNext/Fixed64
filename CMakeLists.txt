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
        
        target_compile_options(${example_name} PRIVATE 
            -Wall 
            -Wextra 
            -O3
        )
    endforeach()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add Berkeley SoftFloat support for benchmarking
option(ENABLE_SOFTFLOAT_BENCHMARK "Enable benchmarking against Berkeley SoftFloat" ON)

if(ENABLE_SOFTFLOAT_BENCHMARK)
    # Define SoftFloat path
    set(SOFTFLOAT_DIR ${PROJECT_SOURCE_DIR}/third_party/berkeley-softfloat-3)
    set(SOFTFLOAT_BUILD_DIR ${SOFTFLOAT_DIR}/build)
    
    # Create specific build directory for the current generator
    set(SOFTFLOAT_SPECIFIC_BUILD_DIR ${SOFTFLOAT_BUILD_DIR}/${CMAKE_GENERATOR})
    file(MAKE_DIRECTORY ${SOFTFLOAT_SPECIFIC_BUILD_DIR})
    
    # Set library file name based on platform
    if(WIN32)
        set(SOFTFLOAT_LIB_FILE "softfloat.lib")
    else()
        set(SOFTFLOAT_LIB_FILE "softfloat.a")
    endif()
    
    # Make sure platform.h exists
    if(NOT EXISTS "${SOFTFLOAT_DIR}/source/include/platform.h")
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/berkeley-softfloat-3/source/include/platform.h
            ${SOFTFLOAT_DIR}/source/include/platform.h
            COPYONLY
        )
    endif()
    
    # Configure and build SoftFloat with the appropriate generator
    add_custom_command(
        OUTPUT ${SOFTFLOAT_SPECIFIC_BUILD_DIR}/${SOFTFLOAT_LIB_FILE}
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" 
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                ${SOFTFLOAT_DIR}
        COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE}
        WORKING_DIRECTORY ${SOFTFLOAT_SPECIFIC_BUILD_DIR}
        COMMENT "Building Berkeley SoftFloat library with ${CMAKE_GENERATOR}"
        VERBATIM
    )
    
    # Create custom target that depends on the library file
    add_custom_target(berkeley_softfloat DEPENDS ${SOFTFLOAT_SPECIFIC_BUILD_DIR}/${SOFTFLOAT_LIB_FILE})
    
    # Create imported library target
    add_library(softfloat STATIC IMPORTED GLOBAL)
    set_target_properties(softfloat PROPERTIES
        IMPORTED_LOCATION ${SOFTFLOAT_SPECIFIC_BUILD_DIR}/${SOFTFLOAT_LIB_FILE}
    )
    add_dependencies(softfloat berkeley_softfloat)
    
    # Set include directories
    set(SOFTFLOAT_INCLUDES 
        ${SOFTFLOAT_DIR}/source/include
        ${SOFTFLOAT_DIR}/source/RISCV
        ${SOFTFLOAT_SPECIFIC_BUILD_DIR}
    )
    
    # Add benchmark directory
    add_subdirectory(benchmarks)
endif()
