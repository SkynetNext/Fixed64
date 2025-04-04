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
