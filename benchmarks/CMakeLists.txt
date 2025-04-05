# Benchmarks for comparing Fixed64 with Berkeley SoftFloat

add_executable(fixed_vs_softfloat
    fixed_vs_softfloat.cpp
)

target_include_directories(fixed_vs_softfloat PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${SOFTFLOAT_INCLUDES}
)

target_link_libraries(fixed_vs_softfloat PRIVATE
    Fixed64
    softfloat
)

# Add compiler options for benchmarks
target_compile_options(fixed_vs_softfloat PRIVATE
    -O3
    -Wall
    -Wextra
)

# New benchmark for Fixed64 advanced math functions
add_executable(fixed64_math_benchmark
    fixed64_math_functions.cpp
)

target_include_directories(fixed64_math_benchmark PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(fixed64_math_benchmark PRIVATE
    Fixed64
)

# Add compiler options for math functions benchmark
target_compile_options(fixed64_math_benchmark PRIVATE
    -O3
    -Wall
    -Wextra
)
