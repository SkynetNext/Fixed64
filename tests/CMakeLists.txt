# Collect all test source files
file(GLOB TEST_SOURCES "*.cpp")

# Create test executable
add_executable(fixed64_tests ${TEST_SOURCES})

# Add Google Test include paths
target_include_directories(fixed64_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include
)

# Link Google Test and your library
target_link_libraries(fixed64_tests
  PRIVATE
    gtest
    gtest_main
    Fixed64
)

# Add compilation options
target_compile_options(fixed64_tests PRIVATE 
    -Wall 
    -Wextra
)

# Use Google Test for test discovery
include(GoogleTest)
gtest_discover_tests(fixed64_tests)