# Approximate Query Engine (AQE)

This project is a C++ implementation of an Approximate Query Engine, designed to process analytical queries over large datasets with a focus on speed over perfect accuracy.

## Features

- SQL-like query parsing (`SELECT`, `FROM`, `GROUP BY`, `SAMPLE`)
- Support for aggregate functions (`COUNT`, `AVG`, `SUM`, `MIN`, `MAX`)
- Multiple sampling strategies:
  - Simple Random
  - Systematic
  - Reservoir
  - Stratified
- Core probabilistic data structures (`CountMinSketch`, `HyperLogLog`, etc.)

## How to Build and Run

### Prerequisites
- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake (version 3.14 or higher)
- Git

### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone <your-repo-url>
    cd ApproximateQueryEngine
    ```

2.  **Configure the project using CMake:**
    ```bash
    cmake -B build
    ```

3.  **Compile the code:**
    ```bash
    cmake --build build
    ```

### Running the Demo

Execute the main application from the project root:
```bash
./build/aqe