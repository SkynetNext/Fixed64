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
    # Define SoftFloat paths
    set(SOFTFLOAT_DIR ${PROJECT_SOURCE_DIR}/third_party/berkeley-softfloat-3)
    
    # Just use pre-built SoftFloat library
    if(WIN32)
        set(SOFTFLOAT_LIB_FILE "${SOFTFLOAT_DIR}/lib/windows/softfloat.a")
    else()
        set(SOFTFLOAT_LIB_FILE "${SOFTFLOAT_DIR}/lib/linux/softfloat.a")
    endif()
    
    # Verify the pre-built library exists
    if(NOT EXISTS "${SOFTFLOAT_LIB_FILE}")
        message(FATAL_ERROR "Pre-built SoftFloat library not found at ${SOFTFLOAT_LIB_FILE}. 
            Please build SoftFloat manually and place the library in the expected location.")
    endif()
    
    # Create imported library target
    add_library(softfloat STATIC IMPORTED GLOBAL)
    set_target_properties(softfloat PROPERTIES
        IMPORTED_LOCATION "${SOFTFLOAT_LIB_FILE}"
    )
    
    # Set include directories
    set(SOFTFLOAT_INCLUDES 
        ${SOFTFLOAT_DIR}/source/include
        ${SOFTFLOAT_DIR}/source/RISCV
    )
    
    # Add benchmark directory
    add_subdirectory(benchmarks)
endif()
