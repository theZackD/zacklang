# ZackLang Language Guide

This guide provides a comprehensive introduction to programming in ZackLang. It's designed to help you get started with the language and understand its core concepts.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Basic Syntax](#basic-syntax)
3. [Variables and Types](#variables-and-types)
4. [Control Flow](#control-flow)
5. [Functions](#functions)
6. [Arrays](#arrays)
7. [String Handling](#string-handling)

## Getting Started

ZackLang is a statically-typed language that emphasizes readability and type safety. Here's a simple "Hello, World!" program:

```zacklang
fn main(): void {
    print("Hello, World!")
}
```

## Basic Syntax

### Comments

```zacklang
// Single line comment

// ZackLang currently only supports
// single-line comments
```

### Statements

- Each statement typically ends with a semicolon (;)
- Blocks are enclosed in curly braces {}
- Indentation is not significant but recommended for readability

## Variables and Types

### Variable Declaration

```zacklang
// Mutable variable
let x: i32 = 42;

// Constant (immutable) variable
let const pi: f64 = 3.14159;

// Type inference
let message = "Hello"; // Inferred as string
```

### Primitive Types

- `i32`: 32-bit integer
- `i64`: 64-bit integer
- `f32`: 32-bit floating-point
- `f64`: 64-bit floating-point
- `bool`: Boolean (true/false)
- `char`: Single character
- `string`: String of characters
- `void`: Used for functions that don't return a value

## Control Flow

### If Statements

```zacklang
if (condition) {
    // code
} elif (other_condition) {
    // code
} else {
    // code
}
```

### Switch Statements

```zacklang
switch (value) {
    case 1:
        print("One");
    case 2:
        print("Two");
    finally:
        print("Done");
}
```

### Loops

#### While Loop

```zacklang
while (condition) {
    // code
    if (exit_condition) {
        break;
    }
    if (skip_condition) {
        continue;
    }
}
```

#### For Loop

```zacklang
// Range-based for loop
for (i in {0:5}) {
    print(f"Index: {i}");
}
```

## Functions

### Basic Function

```zacklang
fn add(a: i32, b: i32): i32 {
    return a + b;
}
```

### Compile-time Function

```zacklang
comptime fn factorial(n: i32): i32 {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}
```

### Function Parameters

- Parameters require type annotations
- Return type is specified after the parameter list
- `void` return type for functions that don't return a value

## Arrays

### Array Declaration

```zacklang
// Array type declaration
let numbers: i32[] = [1, 2, 3, 4, 5];

// Accessing array elements
let first = numbers[0];
```

## String Handling

### String Literals

```zacklang
// Regular string
let message: string = "Hello, World!";

// F-string (formatted string)
let name = "Alice";
let greeting = f"Hello, {name}!";
```

### String Operations

```zacklang
// String concatenation using +
let full_name = first_name + " " + last_name;
```

## Input and Output

### Print Function

```zacklang
print("Basic output");
print(f"Formatted output: {value}");
```

### Prompt Function

```zacklang
let name = prompt("Enter your name: ");
print(f"Hello, {name}!");
```

## Best Practices

1. Use meaningful variable and function names
2. Add comments for complex logic
3. Use constants (`let const`) for values that shouldn't change
4. Properly indent your code for readability
5. Use type annotations for clarity, even when type inference is available
6. Break down complex functions into smaller, reusable functions
7. Use f-strings for string formatting instead of concatenation
8. Handle edge cases in your functions

## Common Patterns

### Error Handling

```zacklang
fn divide(a: f64, b: f64): f64 {
    if (b == 0.0) {
        print("Error: Division by zero");
        return 0.0;
    }
    return a / b;
}
```

### Iterating Over Arrays

```zacklang
let numbers: i32[] = [1, 2, 3, 4, 5];
for (i in {0:5}) {
    print(f"Number at index {i}: {numbers[i]}");
}
```

### Compile-time Calculations

```zacklang
comptime fn power(base: i32, exp: i32): i32 {
    if (exp == 0) {
        return 1;
    }
    return base * power(base, exp - 1);
}

let const SQUARED = power(2, 2);  // Calculated at compile time
```
