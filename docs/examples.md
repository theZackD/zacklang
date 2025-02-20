# ZackLang Examples

This document provides a collection of example programs demonstrating various features and patterns in ZackLang.

## Table of Contents

1. [Basic Examples](#basic-examples)
2. [Control Flow Examples](#control-flow-examples)
3. [Function Examples](#function-examples)
4. [Array Examples](#array-examples)
5. [String Examples](#string-examples)
6. [Compile-time Examples](#compile-time-examples)

## Basic Examples

### Hello World

```zacklang
fn main(): void {
    print("Hello, World!");
}
```

### Basic Arithmetic

```zacklang
fn main(): void {
    let a: i32 = 10;
    let b: i32 = 20;

    print(f"Sum: {a + b}");
    print(f"Difference: {a - b}");
    print(f"Product: {a * b}");
    print(f"Quotient: {a / b}");
    print(f"Power: {a ** 2}");
}
```

### User Input

```zacklang
fn main(): void {
    let name = prompt("What is your name? ");
    print(f"Hello, {name}!");

    let age_str = prompt("What is your age? ");
    let age: i32 = age_str;  // Implicit conversion
    print(f"In 10 years, you will be {age + 10} years old.");
}
```

## Control Flow Examples

### If-Elif-Else Statement

```zacklang
fn grade_score(score: i32): void {
    if (score >= 90) {
        print("Grade: A");
    } elif (score >= 80) {
        print("Grade: B");
    } elif (score >= 70) {
        print("Grade: C");
    } elif (score >= 60) {
        print("Grade: D");
    } else {
        print("Grade: F");
    }
}
```

### Switch Statement

```zacklang
fn day_of_week(day: i32): void {
    switch (day) {
        case 1:
            print("Monday");
        case 2:
            print("Tuesday");
        case 3:
            print("Wednesday");
        case 4:
            print("Thursday");
        case 5:
            print("Friday");
        case 6:
            print("Saturday");
        case 7:
            print("Sunday");
        finally:
            print("End of week check");
    }
}
```

### While Loop

```zacklang
fn countdown(start: i32): void {
    let current: i32 = start;
    while (current > 0) {
        print(f"{current}...");
        current = current - 1;
    }
    print("Liftoff!");
}
```

### For Loop

```zacklang
fn print_squares(): void {
    for (i in {1:10}) {
        print(f"Square of {i} is {i ** 2}");
    }
}
```

## Function Examples

### Basic Function

```zacklang
fn add(a: i32, b: i32): i32 {
    return a + b;
}

fn main(): void {
    let result = add(5, 3);
    print(f"5 + 3 = {result}");
}
```

### Recursive Function

```zacklang
fn factorial(n: i32): i32 {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}
```

### Multiple Return Values (Using Arrays)

```zacklang
fn min_max(numbers: i32[]): i32[] {
    let min: i32 = numbers[0];
    let max: i32 = numbers[0];

    for (i in {1:5}) {
        if (numbers[i] < min) {
            min = numbers[i];
        }
        if (numbers[i] > max) {
            max = numbers[i];
        }
    }

    return [min, max];
}
```

## Array Examples

### Array Operations

```zacklang
fn array_operations(): void {
    // Array declaration and initialization
    let numbers: i32[] = [1, 2, 3, 4, 5];

    // Accessing elements
    let first = numbers[0];
    let last = numbers[4];

    // Modifying elements
    numbers[2] = 10;

    // Array in loop
    for (i in {0:5}) {
        print(f"Element {i}: {numbers[i]}");
    }
}
```

### Array Manipulation

```zacklang
fn double_elements(arr: i32[]): i32[] {
    let result: i32[] = [];
    for (i in {0:5}) {
        result[i] = arr[i] * 2;
    }
    return result;
}
```

## String Examples

### String Manipulation

```zacklang
fn string_examples(): void {
    let name = "Alice";
    let greeting = f"Hello, {name}!";
    print(greeting);

    // String concatenation
    let full_name = "John" + " " + "Doe";
    print(full_name);
}
```

### F-string Usage

```zacklang
fn format_person(name: string, age: i32, city: string): void {
    print(f"Name: {name}, Age: {age}, City: {city}");
    print(f"{name} will be {age + 10} years old in 10 years.");
}
```

## Compile-time Examples

### Compile-time Constants

```zacklang
comptime fn calculate_pi(): f64 {
    return 3.14159265359;
}

let const PI = calculate_pi();

fn circle_area(radius: f64): f64 {
    return PI * radius ** 2;
}
```

### Compile-time Fibonacci

```zacklang
comptime fn fibonacci(n: i32): i32 {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

fn main(): void {
    let const FIB_10 = fibonacci(10);
    print(f"The 10th Fibonacci number is: {FIB_10}");

    // Print first 10 Fibonacci numbers
    for (i in {0:10}) {
        let const fib = fibonacci(i);
        print(f"Fibonacci({i}) = {fib}");
    }
}
```

### Compile-time Power Function

```zacklang
comptime fn power(base: i32, exp: i32): i32 {
    if (exp == 0) {
        return 1;
    }
    return base * power(base, exp - 1);
}

fn main(): void {
    let const POWERS_OF_TWO: i32[] = [
        power(2, 0),
        power(2, 1),
        power(2, 2),
        power(2, 3),
        power(2, 4)
    ];

    for (i in {0:5}) {
        print(f"2^{i} = {POWERS_OF_TWO[i]}");
    }
}
```
