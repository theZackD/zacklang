# ZackLang Language Guide

A hands-on introduction to ZackLang for people who are new to the language. It teaches by example, one idea at a time. For the complete and precise rules, see the [Language Reference](language_reference.md).

> **Status:** ZackLang is being rebuilt from a written specification and the compiler does not exist yet. The examples show how the language is designed to behave, and they cannot be run yet. Anything not described in the docs is marked "not yet specified".

## Contents

1. [Your First Program](#your-first-program)
2. [Variables and Constants](#variables-and-constants)
3. [Types and Conversions](#types-and-conversions)
4. [Operators](#operators)
5. [Making Decisions](#making-decisions)
6. [Loops](#loops)
7. [Functions](#functions)
8. [Arrays](#arrays)
9. [Text: Characters and Strings](#text-characters-and-strings)
10. [Structs](#structs)
11. [Scope](#scope)
12. [Compile-time Functions](#compile-time-functions)
13. [When Things Go Wrong](#when-things-go-wrong)
14. [Input and Output](#input-and-output)
15. [Complete Examples](#complete-examples)
16. [Common Mistakes](#common-mistakes)
17. [What Is Not in the Language Yet](#what-is-not-in-the-language-yet)

## Your First Program

```zacklang
fn main(): void {
    print("Hello, World!");
}
```

- A program starts at the function `main`.
- `fn` declares a function. `: void` says it returns nothing.
- `print` writes a value followed by a newline.
- Every simple statement ends with `;`.

A file may only contain declarations at the top level: functions, structs and constants. Statements such as `print(...)` must live inside a function.

`main` can also return an `i32`, which becomes the program's exit code:

```zacklang
fn main(): i32 {
    print("done");
    return 0;
}
```

**Comments**

```zacklang
// A line comment runs to the end of the line.

/* A block comment can span
   several lines. It cannot be nested. */
```

## Variables and Constants

The snippets from here on show statements; imagine them inside a function such as `main`.

```zacklang
let age = 30;              // a variable; the type (i32) is inferred
let height: f64 = 1.82;    // the type written explicitly
let const PI = 3.14159;    // a constant: it can never change

age = 31;                  // ok: variables can be reassigned
// PI = 3.0;               // error: PI is a constant
```

- `let` creates a variable. `let const` creates a constant that must be given a value when declared.
- The type can be left out when there is a value to infer it from.
- A variable can be declared first and assigned later, but then it needs an explicit type, and the compiler checks that it is assigned before it is used:

```zacklang
let label: string;
if (age >= 18) {
    label = "adult";
} else {
    label = "minor";
}
print(label);              // ok: every path assigns label
```

If one path leaves the variable unassigned, using it afterwards is a compile-time error. There are no hidden default values.

**Updating a variable**

```zacklang
let count = 10;
count = count + 1;
count += 5;                // also -=, *=, /=, %=
```

Assignment is a statement, not an expression, so `a = b = c` and `if (x = 3)` are not valid.

**Constants go all the way down.** A `let const` array or struct cannot be changed either: `NUMBERS[0] = 9;` and `ORIGIN.x = 1.0;` are errors.

## Types and Conversions

### The Primitive Types

| Type | What it holds | Example |
|------|---------------|---------|
| `i32` | 32-bit whole number | `42` |
| `i64` | 64-bit whole number | `5_000_000_000` |
| `f32` | 32-bit decimal number | `1.5` |
| `f64` | 64-bit decimal number | `3.14159` |
| `bool` | true or false | `true` |
| `char` | one Unicode character | `'a'` |
| `string` | text (immutable UTF-8) | `"hello"` |
| `void` | nothing; only as a function's return type | |

### Writing Numbers

```zacklang
42            // decimal
1_000_000     // underscores make long numbers readable
0xFF          // hexadecimal (255)
0b1010        // binary (10)
3.14          // decimal number
.5            // 0.5
1.5e3         // 1500.0
2e-4          // 0.0002
```

`5.` is not valid; write `5.0`. Numbers have no sign of their own: `-5` is the minus operator applied to `5`.

### Inferred Types

With no other information, a whole-number literal is an `i32` and a decimal literal is an `f64`:

```zacklang
let a = 7;        // i32
let b = 7.0;      // f64
let c = 'z';      // char
let d = "hi";     // string
let e = true;     // bool
```

### Number Literals Adapt to Their Context

A literal takes the type the context asks for, as long as the value fits:

```zacklang
let big: i64 = 1;          // ok: the literal becomes an i64
let small: f32 = 1.5;      // ok: the literal becomes an f32
let n = 5;
let m = n + 1;             // ok: the 1 becomes an i32 to match n
// let tooBig: i32 = 3_000_000_000;   // error: does not fit in an i32
// let f: f64 = 1;                    // error: write 1.0 for a decimal number
```

### Converting With `as`

ZackLang never converts between number types behind your back. Say it with `as`:

```zacklang
let total = 17;
let count = 4;
let average = total as f64 / count as f64;    // 4.25
let wide: i64 = total as i64;
let whole = 9.99 as i32;                      // 9 (cuts off the decimals)
let code = 'A' as i32;                        // 65
```

`as` binds tighter than `*` and `/`, so the line computing `average` reads as `(total as f64) / (count as f64)`.

| Conversion | What happens |
|------------|--------------|
| integer to integer | widening keeps the value; narrowing cuts off the high bits |
| integer to decimal | nearest value the decimal type can hold |
| decimal to integer | cuts off the decimals; out-of-range values saturate; `NaN` becomes 0 |
| `bool` to integer | `false` is 0, `true` is 1 |
| `char` to `i32` or `i64` | the character's Unicode number |
| `i32` to `char` | the matching character; panics if the number is not a valid character |

Other conversions are compile-time errors. Turning text into numbers (and back) will be done with library functions (not yet specified).

## Operators

### Arithmetic

```zacklang
let a = 17;
let b = 5;
print(a + b);      // 22
print(a - b);      // 12
print(a * b);      // 85
print(a / b);      // 3     whole-number division cuts off the decimals
print(a % b);      // 2     remainder
print(2 ** 10);    // 1024  power
```

- Both operands must have the same type (a literal adapts to the other side).
- `/` on whole numbers rounds toward zero: `-7 / 2` is `-3`, and `-7 % 2` is `-1`.
- `/` and `%` by zero stop the program (see [When Things Go Wrong](#when-things-go-wrong)).
- `**` is right-to-left: `2 ** 3 ** 2` is `2 ** 9`, which is `512`. It binds tighter than a minus sign in front: `-2 ** 2` is `-4`.
- A whole number raised to a negative whole number is an error.

### Comparison

```zacklang
a == b      a != b      a < b      a <= b      a > b      a >= b
```

Both sides must have the same type and the result is a `bool`. `==` and `!=` work on every type: strings, arrays and structs compare by their contents. You cannot chain comparisons: `1 < x < 10` is an error. Write `1 < x and x < 10`.

### Logic

```zacklang
let ok = age >= 18 and not banned;
let either = a or b;
let different = a xor b;      // true when exactly one is true
```

Logic works only on `bool`s. `and` and `or` skip the right side when the left side already decides the answer, so this is safe:

```zacklang
if (i < 10 and values[i] > 0) { ... }    // values[i] is not read when i >= 10
```

### Order of Evaluation

From tightest to loosest: calls, `[]` and `.`, then `**`, then unary `-` and `not`, then `as`, then `* / %`, then `+ -`, then `< <= > >=`, then `== !=`, then `and`, then `xor`, then `or`. When in doubt, add parentheses.

## Making Decisions

### if, elif, else

```zacklang
if (score >= 90) {
    print("A");
} elif (score >= 80) {
    print("B");
} else {
    print("C or below");
}
```

The condition goes in parentheses and the braces are required. The condition must be a `bool`; numbers and strings are not "truthy".

### switch

`switch` picks one branch by value:

```zacklang
let name: string;
switch (day) {
    case 1:
        name = "Monday";
    case 2:
        name = "Tuesday";
    case 6, 7:
        name = "Weekend";
    finally:
        name = "Some other day";
}
print(name);
```

- Only one branch runs. There is **no fall-through** and no `break` is needed.
- `case 6, 7:` matches several values.
- `finally:` is the default branch and runs when nothing else matched. It is optional and must come last.
- You can switch on integers, `char`s, `bool`s and `string`s. Case values must be constants and may not repeat.
- Each branch is its own scope.

## Loops

### while

```zacklang
let n = 1;
while (n <= 100) {
    n *= 2;
}
print(n);          // 128
```

### for

```zacklang
for (i in {0:5}) {
    print(i);                 // 0 1 2 3 4
}
```

`{0:5}` counts from 0 up to **but not including** 5. If the start is not below the end, the loop does not run. The loop variable (`i`) belongs to the loop and cannot be changed inside it.

To visit every element of an array:

```zacklang
let primes = [2, 3, 5, 7];
for (p in primes) {
    print(p);
}
```

### break and continue

```zacklang
for (i in {0:100}) {
    if (i % 2 == 0) { continue; }     // skip even numbers
    if (i > 7) { break; }             // stop the loop
    print(i);                         // 1 3 5 7
}
```

`break` and `continue` always refer to the innermost loop, even when they appear inside a `switch`.

## Functions

```zacklang
fn add(a: i32, b: i32): i32 {
    return a + b;
}

fn greet(name: string): void {
    print(f"Hello, {name}!");
}

fn main(): void {
    print(add(2, 3));
    greet("Ada");
}
```

- Parameter types and the return type are always written out, even for `void`.
- Arguments are passed by value, and parameters cannot be changed inside the function.
- Every path through a function that returns a value must end with `return`. The compiler checks this.
- Functions can be used before the place where they are written, and can call themselves or each other:

```zacklang
fn is_even(n: i32): bool {
    if (n == 0) { return true; }
    return is_odd(n - 1);
}

fn is_odd(n: i32): bool {
    if (n == 0) { return false; }
    return is_even(n - 1);
}
```

- There is no overloading and no default arguments, and functions cannot yet be stored in variables or passed to other functions.

## Arrays

### Fixed-size Arrays

```zacklang
let days: i32[3] = [10, 20, 30];     // exactly 3 elements
let first = days[0];                 // 10; counting starts at 0
days[2] = 99;
print(len(days));                    // 3
```

`i32[3]` is a value: copying it copies all three elements. The size must be a constant known when compiling, such as `3` or a `let const`.

### Dynamic Arrays

```zacklang
let scores: i32[] = [88, 92, 79];
print(len(scores));                  // 3
```

`i32[]` is an array whose length can change at run time. The operations for growing and shrinking (such as adding an element) will be part of the standard library, which is not specified yet.

### Arrays of Arrays

The sizes are written outermost first, so `i32[2][3]` is 2 rows of 3 columns:

```zacklang
let grid: i32[2][3] = [[1, 2, 3],
                       [4, 5, 6]];
print(grid[1][2]);                   // 6: row 1, column 2
grid[0][1] = 20;
```

### Safety

Every index is checked. Using an index that is out of range stops the program with a clear error instead of reading some unrelated memory.

An array literal takes the type the context expects, so `let a: i32[] = [];` works. With nothing to go on, `[1, 2, 3]` is a fixed `i32[3]`. All elements must have the same type, and a fixed array and a dynamic array are not converted into each other automatically.

## Text: Characters and Strings

```zacklang
let letter: char = 'a';
let message: string = "Hello, World!";
let both = "She said \"hi\"\nand left.";
let heart = '\u{2764}';
```

| Escape | Meaning |
|--------|---------|
| `\n` `\t` `\r` | new line, tab, carriage return |
| `\\` `\"` `\'` | backslash, double quote, single quote |
| `\0` | the null character |
| `\u{XXXX}` | any Unicode character by its hex code (1 to 6 digits) |

Strings are UTF-8 and cannot be changed after they are created. A string or character literal cannot continue onto a second line; use `\n`.

### f-strings

Put an `f` in front of a string to insert values with `{ }`:

```zacklang
let name = "Ada";
let year = 1815;
print(f"{name} was born in {year}.");       // Ada was born in 1815.
print(f"2 + 3 = {2 + 3}");                  // any expression works
print(f"Use {{braces}} like this");         // Use {braces} like this
```

An f-string always produces a `string`. Joining strings with `+` and other string operations will come with the standard library (not yet specified).

Strings compare by content (`==`, `<`, ...), and `len("héllo")` counts characters (here 5), not bytes.

## Structs

A struct groups related values under one name:

```zacklang
struct Rect {
    width: i32,
    height: i32,
}

fn area(r: Rect): i32 {
    return r.width * r.height;
}

fn main(): void {
    let r = Rect { width: 3, height: 4 };
    print(area(r));              // 12

    r.width = 10;                // fields can change if r is not a constant
    print(area(r));              // 40
}
```

- Every field must be given when a struct is created.
- Structs are copied on assignment and when passed to a function; changing a copy does not change the original.
- A struct may hold arrays or other structs, but not itself.
- Two structs are equal with `==` when all their fields are equal.

## Scope

A variable lives from its declaration to the end of the block `{ }` that contains it.

```zacklang
let x = 1;
{
    let x = 2;        // a new variable that hides the outer x
    print(x);         // 2
}
print(x);             // 1
```

- An inner block may reuse a name from an outer block (this is called shadowing).
- Declaring the same name twice in the same block is an error. A function's parameters count as part of the function's outermost block, so a parameter's name cannot be redeclared at the top of the body.
- Functions, structs and top-level constants can be used anywhere in the file.

## Compile-time Functions

Mark a function `comptime` and the compiler can run it while compiling, so the answer is already in the program:

```zacklang
comptime fn square(n: i32): i32 {
    return n * n;
}

let const AREA = square(12);         // computed during compilation: 144
let const SIZE = 3;                  // array sizes must be known at compile time

fn main(): void {
    let grid: i32[SIZE][SIZE] = [[0, 0, 0], [0, 0, 0], [0, 0, 0]];
    print(AREA);                     // 144
    print(square(7));                // 49
}
```

Rules for `comptime` functions:

- They must be pure: no printing, reading input, or touching anything outside the function.
- They can use arithmetic, comparisons, `if`, `while`, `for`, local variables, fixed-size arrays, structs and calls to other `comptime` functions.
- A call with constant arguments is evaluated by the compiler. With non-constant arguments the function simply runs at run time like any other.
- Compile-time evaluation has a step limit and a recursion limit. Exceeding either is a compile-time error, so a `comptime` function can never hang the compiler.
- A top-level `let const` must be computable at compile time, as `AREA` is.

## When Things Go Wrong

**Compile-time errors** stop the build before the program exists. Examples: a type mismatch, an undeclared name, a missing `return`, assigning to a constant, an unassigned variable, a literal that does not fit.

**Panics** happen while the program runs. A panic prints a message with the source line to standard error and exits with code 101. They cannot be caught. Panics are caused by:

- an array index out of range
- dividing by zero (`/` or `%`)
- integer overflow, **in debug builds only** (release builds wrap around)
- a whole number raised to a negative power
- converting an invalid number to `char` with `as`
- calling `panic("message")` yourself

```zacklang
fn get(values: i32[3], i: i32): i32 {
    return values[i];            // panics when i is 3 or more
}
```

## Input and Output

```zacklang
fn main(): void {
    let name = prompt("Your name: ");      // shows the message, reads one line
    print(f"Nice to meet you, {name}!");
}
```

| Built-in | What it does |
|----------|--------------|
| `print(value)` | writes any printable value and a newline |
| `prompt(message): string` | writes `message`, reads a line, returns it without the line break |
| `panic(message)` | stops the program with a panic |
| `len(x): i64` | length of an array, or the number of characters in a string |

`prompt` always returns text. Turning it into a number needs a library function (not yet specified). The exact text printed for decimal numbers is not yet specified.

## Complete Examples

### FizzBuzz

```zacklang
fn main(): void {
    for (i in {1:16}) {
        if (i % 15 == 0) {
            print("FizzBuzz");
        } elif (i % 3 == 0) {
            print("Fizz");
        } elif (i % 5 == 0) {
            print("Buzz");
        } else {
            print(i);
        }
    }
}
```

### Counting Primes

```zacklang
fn is_prime(n: i32): bool {
    if (n < 2) {
        return false;
    }
    let d = 2;
    while (d * d <= n) {
        if (n % d == 0) {
            return false;
        }
        d += 1;
    }
    return true;
}

fn main(): i32 {
    let count = 0;
    for (n in {0:100}) {
        if (is_prime(n)) {
            count += 1;
        }
    }
    print(f"There are {count} primes below 100.");    // 25
    return 0;
}
```

### Summing an Array

```zacklang
fn sum(values: i32[]): i32 {
    let total = 0;
    for (v in values) {
        total += v;
    }
    return total;
}

fn main(): void {
    print(sum([4, 8, 15, 16, 23, 42]));       // 108
}
```

## Common Mistakes

| Mistake | What happens | Fix |
|---------|--------------|-----|
| `let x: i64 = n;` where `n` is an `i32` | compile error: no automatic conversion | `n as i64` |
| `let f: f64 = 1;` | compile error: a whole-number literal is not a decimal | `1.0` |
| `if (count) { ... }` | compile error: the condition must be a `bool` | `if (count != 0)` |
| `1 < x < 10` | compile error: comparisons do not chain | `1 < x and x < 10` |
| `a = b = 0;` | compile error: assignment is not an expression | two separate statements |
| `case 1:` hoping to run the next case too | there is no fall-through | list the values: `case 1, 2:` |
| `fn f(a, b) { ... }` | compile error: types are required | `fn f(a: i32, b: i32): void` |
| using a variable that may not be assigned | compile error | give it a value on every path, or at declaration |
| `x / 0` | panic at run time | check the divisor first |
| `-2 ** 2` expecting `4` | it is `-4` | `(-2) ** 2` |

## What Is Not in the Language Yet

The language is designed in stages. These are not available or not specified yet:

- Joining strings with `+`, and other string operations
- Adding and removing elements of dynamic arrays, and other array helpers
- Converting between text and numbers (`parse_i32`, `to_string`, ...)
- Modules and imports, enums, generics
- Functions as values, and closures
- Pointers and references
- Error handling other than panics
- Files and other I/O beyond `print` and `prompt`

See [DECISIONS.md](DECISIONS.md) for the current list of open design questions.

## Next Steps

- Read the [Language Reference](language_reference.md) for the exact rules.
- Read the [Style Guide](style_guide.md) for formatting conventions.
- Browse [examples.md](examples.md) for more programs.
