# Performance Benchmarks

Comprehensive benchmarks comparing Fixed64 with [soft_double](https://github.com/ckormanyos/soft_double.git) (software floating point) and hardware floating point (double).

## Basic Arithmetic Operations

| Operation      | Fixed64 (ms) | SoftDouble(ms) | Speedup vs SoftDouble | double (ms) | Speedup vs double |
|----------------|--------------|----------------|-----------------------|-------------|-------------------|
| Addition       | 7.52         | 104.17         | 13.84x                | 16.64       | 2.21x             |
| Subtraction    | 4.16         | 101.51         | 24.39x                | 16.02       | 3.85x             |
| Multiplication | 20.40        | 173.04         | 8.48x                 | 12.67       | 0.62x             |
| Division       | 212.34       | 299.60         | 1.41x                 | 14.83       | 0.07x             |
| Square Root    | 202.36       | 229.31         | 1.13x                 | 20.57       | 0.10x             |

## Advanced Mathematical Functions

| Function | Fixed64 (ms) | SoftDouble (ms) | Speedup vs SoftDouble | double (ms) | Speedup vs double |
|----------|--------------|-----------------|------------------------|-------------|-------------------|
| Pow2     | 131.40       | 2677.72         | 20.38x                 | 542.61      | 4.13x             |
| Exp      | 264.60       | 1634.88         | 6.18x                  | 45.73       | 0.17x             |
| Log      | 338.02       | 1934.12         | 5.72x                  | 53.30       | 0.16x             |
| Pow      | 667.56       | 3711.60         | 5.56x                  | 242.55      | 0.36x             |
| Atan2    | 299.25       | N/A             | N/A                    | 416.03      | 1.39x             |

## Trigonometric Functions

| Function | Implementation | Fixed64 (ms) | SoftDouble (ms) | Speedup vs SoftDouble | double (ms) | Speedup vs double |
|----------|---------------|--------------|-----------------|------------------------|-------------|-------------------|
| Sin      | Standard      | 127.32       | 1879.43         | 14.76x                 | 88.99       | 0.70x             |
|          | Fast          | 49.30        | 1897.93         | 38.49x                 | 91.08       | 1.85x             |
| Tan      | Standard      | 237.73       | 3860.05         | 16.24x                 | 110.80      | 0.47x             |
|          | Fast          | 88.20        | 3962.70         | 44.93x                 | 113.34      | 1.28x             |
| Acos     | Standard      | 70.46        | 2237.73         | 31.76x                 | 187.48      | 2.66x             |
| Atan     | Standard      | 130.52       | 1894.74         | 14.52x                 | 180.56      | 1.38x             |
|          | Fast          | 51.39        | 1951.46         | 37.98x                 | 184.54      | 3.59x             |

## Performance Analysis

1. Fixed64 significantly outperforms SoftDouble across all operations
2. Fixed64 matches or exceeds hardware double precision for many functions
3. Fast implementations of trigonometric functions provide substantial performance improvements
4. Hardware floating point remains faster for some complex operations (Exp, Log, Pow) 