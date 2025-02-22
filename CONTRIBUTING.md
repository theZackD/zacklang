# Contributing to Zacklang

Thank you for your interest in contributing to Zacklang! We welcome contributions from everyone who wishes to improve the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Process](#development-process)
- [Pull Request Process](#pull-request-process)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Documentation](#documentation)

## Code of Conduct

We are committed to providing a friendly, safe, and welcoming environment for all contributors. Please be respectful and constructive in your communications with others.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/yourusername/zacklang.git`
3. Create a new branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Run tests: `make test`
6. Commit your changes: `git commit -m "Description of changes"`
7. Push to your fork: `git push origin feature/your-feature-name`
8. Create a Pull Request

## Development Process

### Setting Up Development Environment

```bash
# Install dependencies
sudo apt-get install llvm-15-dev  # For Ubuntu/Debian
brew install llvm@15             # For macOS

# Build the project
make
```

### Project Structure

- `src/`: Source files
- `include/`: Header files
- `tests/`: Test files
- `docs/`: Documentation

## Pull Request Process

1. Ensure your code follows our coding standards
2. Add tests for any new functionality
3. Update documentation as needed
4. Ensure all tests pass
5. Request review from maintainers

### Commit Message Format

```
type(scope): Description

[optional body]

[optional footer]
```

Types:

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation
- `style`: Formatting
- `refactor`: Code restructuring
- `test`: Adding tests
- `chore`: Maintenance

## Coding Standards

### C++ Style Guide

- Use C++17 features appropriately
- Follow LLVM coding standards
- Use meaningful variable and function names
- Keep functions focused and concise
- Document complex algorithms
- Use consistent indentation (4 spaces)

### Header Files

```cpp
#ifndef ZACKLANG_COMPONENT_NAME_H
#define ZACKLANG_COMPONENT_NAME_H

// Code here

#endif // ZACKLANG_COMPONENT_NAME_H
```

## Testing Guidelines

### Writing Tests

- Write unit tests for new functionality
- Include performance tests for optimizations
- Test edge cases and error conditions
- Use meaningful test names

### Running Tests

```bash
# Run all tests
make test

# Run specific test
./test_component_name
```

### Benchmarking

- Include benchmarks for performance-critical code
- Compare against baseline measurements
- Document methodology and results

## Documentation

### Code Documentation

- Use clear and concise comments
- Document public APIs thoroughly
- Include examples where appropriate
- Keep documentation up-to-date with code changes

### Optimization Documentation

For optimization passes:

1. Describe the optimization
2. Document preconditions and postconditions
3. Include complexity analysis
4. Provide examples
5. Add benchmark results

## Questions or Need Help?

Feel free to:

- Open an issue for questions
- Join our development discussions
- Contact the maintainers

Thank you for contributing to Zacklang!
