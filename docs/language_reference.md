# ZackLang Language Reference

This document provides a detailed technical specification of the ZackLang programming language.

## Table of Contents

1. [Lexical Structure](#lexical-structure)
2. [Types](#types)
3. [Expressions](#expressions)
4. [Statements](#statements)
5. [Functions](#functions)
6. [Compile-time Features](#compile-time-features)

## Lexical Structure

### Keywords

The following are reserved keywords in ZackLang:

```
let       const     print     prompt    if
else      elif      case      switch    finally
true      false     fn        return    break
continue  while     for       and       or
not       xor       in        struct    i32
i64       f32       f64       bool      char
string    void      comptime
```

### Operators

- **Arithmetic**: `+`, `-`, `*`, `/`, `%`, `**` (power)
- **Comparison**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Logical**: `and`, `or`, `not`, `xor`
- **Assignment**: `=`

### Literals

1. **Integer Literals**
   - Decimal: `0`, `123`, `-456`
2. **Floating-Point Literals**
   - Decimal: `0.0`, `3.14`, `-0.001`
3. **String Literals**
   - Regular strings: `"Hello, World!"`
   - F-strings: `f"Value: {expression}"`
4. **Boolean Literals**
   - `true`, `false`

### Identifiers

- Begin with a letter or underscore
- Can contain letters, digits, and underscores
- Case-sensitive
- Cannot be a keyword

## Types

### Primitive Types

1. **Integer Types**
   - `i32`: 32-bit signed integer
   - `i64`: 64-bit signed integer
2. **Floating-Point Types**
   - `f32`: 32-bit floating-point
   - `f64`: 64-bit floating-point
3. **Boolean Type**
   - `bool`: true or false
4. **Character Type**
   - `char`: Single Unicode character
5. **String Type**
   - `string`: Sequence of characters
6. **Void Type**
   - `void`: Used for functions with no return value

### Array Types

- Denoted by `type[]`
- Example: `i32[]` for array of 32-bit integers
- Zero-based indexing

### Type Inference

- Types can be inferred in variable declarations
- Function parameters and return types must be explicitly annotated

## Expressions

### Arithmetic Expressions

- Addition: `a + b`
- Subtraction: `a - b`
- Multiplication: `a * b`
- Division: `a / b`
- Modulo: `a % b`
- Power: `a ** b`

### Comparison Expressions

- Equal: `a == b`
- Not Equal: `a != b`
- Less Than: `a < b`
- Greater Than: `a > b`
- Less Than or Equal: `a <= b`
- Greater Than or Equal: `a >= b`

### Logical Expressions

- AND: `a and b`
- OR: `a or b`
- NOT: `not a`
- XOR: `a xor b`

### Function Calls

```zacklang
function_name(arg1, arg2, ...)
```

### Array Indexing

```zacklang
array[index]
```

## Statements

### Variable Declaration

```zacklang
let [const] identifier [: type] = expression;
```

### If Statement

```zacklang
if (condition) {
    // statements
} elif (condition) {
    // statements
} else {
    // statements
}
```

### Switch Statement

```zacklang
switch (expression) {
    case value1:
        // statements
    case value2:
        // statements
    finally:
        // statements
}
```

### While Loop

```zacklang
while (condition) {
    // statements
}
```

### For Loop

```zacklang
for (identifier in {start:end}) {
    // statements
}
```

### Return Statement

```zacklang
return [expression];
```

### Break Statement

```zacklang
break;
```

### Continue Statement

```zacklang
continue;
```

## Functions

### Function Declaration

```zacklang
[comptime] fn identifier(param1: type1, param2: type2): return_type {
    // body
}
```

### Function Parameters

- Must have type annotations
- Pass by value semantics

### Return Values

- Must match declared return type
- `void` functions can omit return statement

## Compile-time Features

### Compile-time Functions

- Marked with `comptime` keyword
- Evaluated during compilation
- Must be pure functions
- Can be used in constant expressions

### Compile-time Evaluation Rules

1. All arguments must be known at compile time
2. No side effects allowed
3. Must terminate
4. Can only call other compile-time functions

### Examples

```zacklang
comptime fn factorial(n: i32): i32 {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

let const FACT_5 = factorial(5);  // Evaluated at compile time
```

## Standard Library

### Input/Output

- `print(expression)`: Output to console
- `prompt(message)`: Get user input

### Type Conversion

(To be implemented)

### String Operations

(To be implemented)

### Array Operations

(To be implemented)
