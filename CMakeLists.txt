cmake_minimum_required(VERSION 3.10)
project(Fixed64 VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use Clang compiler
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Add include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# 修改：只添加头文件库，不需要源文件
add_library(Fixed64 INTERFACE)
target_include_directories(Fixed64 INTERFACE ${PROJECT_SOURCE_DIR}/include)


# Create example programs (if example code exists)
if(EXISTS "${PROJECT_SOURCE_DIR}/examples")
    file(GLOB EXAMPLE_SOURCES "examples/*.cpp")
    foreach(example_source ${EXAMPLE_SOURCES})
        get_filename_component(example_name ${example_source} NAME_WE)
        add_executable(${example_name} ${example_source})
        target_link_libraries(${example_name} Fixed64)
        
        # 为每个示例程序添加编译选项
        target_compile_options(${example_name} PRIVATE 
            -Wall 
            -Wextra 
            -O3
        )
    endforeach()
endif()