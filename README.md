# ZackLang

ZackLang is a modern, statically-typed programming language implemented in C. It combines features from various programming languages while maintaining simplicity and expressiveness.

## Features

- **Static Typing**: Built-in support for various primitive types including:

  - Integer types (`i32`, `i64`)
  - Floating-point types (`f32`, `f64`)
  - Boolean type (`bool`)
  - String type (`string`)
  - Character type (`char`)
  - Void type (`void`)

- **Modern String Handling**:

  - Regular string literals
  - F-strings (formatted strings) with expression interpolation
  - Example: `f"The value is: {expression}"`

- **Control Flow**:

  - If-elif-else statements
  - Switch-case statements with finally clause
  - While loops
  - For loops with range iteration
  - Break and continue statements

- **Functions**:

  - Regular function definitions
  - Compile-time function evaluation with `comptime` modifier
  - Type annotations for parameters and return types
  - Example:
    ```
    comptime fn fibonacci(n: i32): i32 {
        if (n <= 1) {
            return n
        }
        return fibonacci(n - 1) + fibonacci(n - 2)
    }
    ```

- **Variables**:

  - Mutable variables with `let`
  - Constants with `let const`
  - Type inference with explicit type annotations
  - Example: `let const pi: f64 = 3.14159`

- **Arrays**:

  - Array type support with `type[]` syntax
  - Array literals
  - Array indexing

- **Built-in Functions**:

  - `print()` for output
  - `prompt()` for input

- **Operators**:
  - Arithmetic: `+`, `-`, `*`, `/`, `%`, `**` (power)
  - Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
  - Logical: `and`, `or`, `not`, `xor`
  - Assignment: `=`

## Example Program

```zacklang
fn main(): void {
    // Calculate first 5 fibonacci numbers
    let results: i32[] = [fibonacci(0), fibonacci(1),
                         fibonacci(2), fibonacci(3),
                         fibonacci(4)]

    for (i in {0:5}) {
        print(f"Fibonacci({i}) = {results[i]}")
    }
}

comptime fn fibonacci(n: i32): i32 {
    if (n <= 1) {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}
```

## Project Structure

The compiler is implemented with the following components:

- **Lexer**: Tokenizes source code into a stream of tokens
- **Parser**: Builds an Abstract Syntax Tree (AST) from tokens
- **Semantic Analyzer**: Performs type checking and semantic validation
- **Compile-time Evaluator**: Handles compile-time function execution
- **Code Generator**: (In development)

## Building and Running

(Add build instructions and dependencies here when ready)

## Contributing

This is a work in progress. Contributions are welcome! Please feel free to submit a Pull Request.

## License

(Add license information here)
