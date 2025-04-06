# Benchmarks for comparing Fixed64 with Berkeley SoftFloat

# Shared benchmark utilities and implementations
set(BENCHMARK_SOURCES
    benchmark_utils.h
    basic_operations_benchmark.h
    basic_operations_benchmark.cpp
    advanced_math_benchmark.h
    advanced_math_benchmark.cpp
    benchmark_main.cpp
)

# Main benchmark executable
add_executable(fixed64_benchmark ${BENCHMARK_SOURCES})

target_include_directories(fixed64_benchmark PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${SOFTFLOAT_INCLUDES}
)

target_link_libraries(fixed64_benchmark PRIVATE
    Fixed64
)

# Add compiler options for benchmarks
target_compile_options(fixed64_benchmark PRIVATE ${COMPILER_WARNINGS})
