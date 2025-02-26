![Zacklang Logo](logo.png)

# ZackLang Compiler

A modern, optimizing compiler infrastructure designed for performance and reliability.

## Overview

Zacklang is a robust compiler infrastructure that implements advanced optimization techniques and generates efficient LLVM IR. It features a modular design with a focus on extensibility and maintainability.

## Features

- **Advanced Optimization Pipeline**

  - Dead Block Elimination
  - Block Merging ✅
  - Jump Threading (Planned)
  - Critical Edge Splitting (Planned)
  - Value Numbering (Planned)

- **Robust IR System**

  - Type-safe intermediate representation
  - SSA form
  - Comprehensive control flow analysis

- **Performance-Focused Design**
  - Efficient memory management
  - Fast optimization passes
  - Scalable to large codebases

## Performance Benchmarks

Recent benchmarks demonstrate the efficiency of our optimization passes:

### Dead Block Elimination Performance

```
Structure Time (μs) Blocks
Linear 991.39 100,000
Tree 1005.36 100,000
```

### Block Merging Performance

```
Structure    Size    Merge Check (μs)    Merge Time (μs)    Total (μs)
Linear Chain 10      1                   33                 34
Linear Chain 50      3                   90                 93
Linear Chain 100     6                   120                126
Diamond      20      8                   65                 73
Diamond      40      15                  110                125
```

Key findings:

- Dead Block Elimination: Processes 100K blocks in ~1ms
- Block Merging:
  - Scales linearly with graph size
  - Efficient merge checks (< 10μs for most cases)
  - C API implementation performs on par with native C++
  - Handles complex control flow structures efficiently

## Building

```bash
# Clone the repository
git clone https://github.com/yourusername/zacklang.git
cd zacklang

# Build the project
make

# Run tests
make test
```

## Requirements

- C++17 compatible compiler
- LLVM 15.0 or later
- CMake 3.15 or later

## Project Structure

```
zacklang/
├── include/         # Public headers
├── src/            # Implementation files
├── tests/          # Test suite
│   ├── ast/        # AST tests
│   ├── zir/        # ZIR tests
│   └── benchmarks/ # Performance tests
└── docs/           # Documentation
```

## Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details.

## Optimization Passes

### 1. Dead Block Elimination ✅

- Identifies and removes unreachable blocks
- Maintains control flow graph integrity
- Handles complex nested structures
- Benchmark-proven performance

### 2. Block Merging ✅

- Identifies and combines adjacent blocks with safe control flow
- Performs comprehensive safety checks before merging
- Preserves semantic equivalence of the program
- Optimizes instruction layout for better performance
- Reduces branch overhead in generated code
- C API support for integration with external tools
- Thoroughly tested with a variety of control flow patterns

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

Special thanks to all contributors and the LLVM community for their invaluable work and inspiration.

---

_Zacklang - Building the future of compilation, one optimization at a time._
