# ZackLang Style Guide

This style guide provides conventions and best practices for writing clean, readable, and maintainable ZackLang code.

## Table of Contents

1. [Formatting](#formatting)
2. [Naming Conventions](#naming-conventions)
3. [Comments](#comments)
4. [Code Organization](#code-organization)
5. [Best Practices](#best-practices)

## Formatting

### Indentation

- Use 4 spaces for indentation
- Do not use tabs
- Indent inside blocks, including:
  - Function bodies
  - Control flow blocks
  - Array literals spanning multiple lines

```zacklang
// Good
fn calculate_sum(numbers: i32[]): i32 {
    let sum: i32 = 0;
    for (i in {0:5}) {
        sum = sum + numbers[i];
    }
    return sum;
}

// Bad - inconsistent indentation
fn calculate_sum(numbers: i32[]): i32 {
  let sum: i32 = 0;
   for (i in {0:5}) {
      sum = sum + numbers[i];
    }
  return sum;
}
```

### Line Length

- Keep lines under 80 characters when possible
- Break long lines at logical points
- Align continued lines with 8 spaces

```zacklang
// Good
let result = very_long_function_name(
        first_parameter,
        second_parameter,
        third_parameter);

// Bad - line too long
let result = very_long_function_name(first_parameter, second_parameter, third_parameter);
```

### Spacing

- Place one space after keywords
- Place one space around operators
- No space between function name and opening parenthesis
- No space inside parentheses
- One space after commas in parameter lists

```zacklang
// Good
if (condition) {
    let x = a + b;
    function_call(x, y);
}

// Bad
if(condition){
    let x=a+b;
    function_call (x,y);
}
```

## Naming Conventions

### Functions

- Use snake_case for function names
- Names should be verbs or verb phrases
- Compile-time functions should have descriptive names indicating their purpose

```zacklang
// Good
fn calculate_average(numbers: i32[]): f64 {
    // implementation
}

comptime fn factorial(n: i32): i32 {
    // implementation
}

// Bad
fn Avg(numbers: i32[]): f64 {
    // implementation
}
```

### Variables

- Use snake_case for variable names
- Choose descriptive names that indicate purpose
- Constants should be in SCREAMING_SNAKE_CASE

```zacklang
// Good
let user_age: i32 = 25;
let const MAX_ATTEMPTS: i32 = 3;

// Bad
let a: i32 = 25;
let const maxAttempts: i32 = 3;
```

### Type Annotations

- Always include type annotations for function parameters
- Include type annotations for variables when type is not obvious
- Type annotations can be omitted when type inference is clear

```zacklang
// Good
let count: i32 = 0;
let message = "Hello";  // Type is clearly string

// Bad
let value = get_complex_value();  // Type unclear, annotation needed
```

## Comments

### Usage

- Write comments that explain "why" not "what"
- Use comments for complex algorithms
- Avoid redundant comments that just repeat the code
- Document public functions with a brief description

```zacklang
// Good
// Binary search implementation for sorted arrays
fn binary_search(array: i32[], target: i32): i32 {
    // implementation
}

// Bad
// Sets x to 5
let x = 5;
```

### Format

- Start comments with a space after //
- Use complete sentences
- Keep comments concise and relevant

```zacklang
// Good
// Calculate average to determine threshold

// Bad
//no space after comment
//incomplete sentence
```

## Code Organization

### File Structure

- One main function per file
- Related functions should be grouped together
- Constants should be declared at the top of the file
- Implement complex functionality in separate functions

```zacklang
// Constants
let const MAX_SIZE: i32 = 100;
let const DEFAULT_VALUE: f64 = 0.0;

// Helper functions
fn helper_function(): void {
    // implementation
}

// Main function
fn main(): void {
    // implementation
}
```

### Function Organization

- Keep functions focused and small
- Order parameters logically
- Group related parameters
- Return early for error conditions

```zacklang
// Good
fn process_data(data: i32[], size: i32): bool {
    if (size <= 0) {
        return false;
    }
    // Normal processing
    return true;
}
```

## Best Practices

### Type Safety

- Use appropriate types for data
- Avoid type casting when possible
- Use constants for fixed values
- Validate function inputs

### Error Handling

- Check for error conditions early
- Provide meaningful error messages
- Use appropriate return types for error cases

```zacklang
fn divide(a: f64, b: f64): f64 {
    if (b == 0.0) {
        print("Error: Division by zero");
        return 0.0;
    }
    return a / b;
}
```

### Performance

- Use compile-time functions for constant computations
- Avoid unnecessary array copies
- Keep critical loops simple
- Consider memory usage in array operations

### Maintainability

- Break complex operations into smaller functions
- Use meaningful names for clarity
- Document non-obvious code
- Keep functions focused on a single task

### Testing

- Write testable functions
- Keep side effects minimal
- Make dependencies explicit
- Use assertions for invariants

```zacklang
// Good - testable function
fn add_numbers(a: i32, b: i32): i32 {
    return a + b;
}

// Bad - hard to test due to side effects
fn add_and_print(a: i32, b: i32): void {
    print(f"{a} + {b} = {a + b}");
}
```
