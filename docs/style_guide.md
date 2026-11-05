# ZackLang Style Guide

Conventions for writing clear, consistent ZackLang code. They are recommendations for people writing ZackLang, not rules the compiler enforces. The language rules are in the [Language Reference](language_reference.md), and complete programs written in this style are in [examples.md](examples.md).

## Contents

1. [Formatting](#formatting)
2. [Naming](#naming)
3. [Types and Literals](#types-and-literals)
4. [Operators and Conversions](#operators-and-conversions)
5. [Control Flow](#control-flow)
6. [Functions](#functions)
7. [Structs and Arrays](#structs-and-arrays)
8. [Comments](#comments)
9. [File Organization](#file-organization)
10. [Errors and Safety](#errors-and-safety)

## Formatting

### Indentation and Braces

- Indent with 4 spaces; never use tabs.
- Put the opening brace on the same line as the statement, and the closing brace on its own line.
- The language requires braces around the bodies of `if`, `elif`, `else`, `while` and `for`, so always write them. `switch` branches have no braces; put each branch's statements on their own indented lines.
- `elif` and `else` go on the same line as the preceding closing brace.

```zacklang
// Good
fn clamp(value: i32, low: i32, high: i32): i32 {
    if (value < low) {
        return low;
    } elif (value > high) {
        return high;
    } else {
        return value;
    }
}

// Bad: inconsistent indentation and brace placement
fn clamp(value: i32, low: i32, high: i32): i32
{
  if (value < low) { return low; }
   elif (value > high) {
      return high; }
  else { return value; }
}
```

### Line Length

- Keep lines to 100 characters or fewer, and prefer 80.
- Break long lines at commas or operators and indent the continuation by 8 spaces.

```zacklang
// Good
let distance = calculate_distance(
        first_point,
        second_point,
        scale_factor);

// Bad
let distance = calculate_distance(first_point, second_point, scale_factor, offset, rounding_mode);
```

### Spacing

- One space after keywords: `if (`, `while (`, `for (`, `switch (`.
- One space around binary operators and after commas.
- No space between a function name and its `(`, and no space just inside parentheses or brackets.
- One space after `:` in type annotations and struct fields; none before it.
- No space around the range `:` in `{start:end}`.

```zacklang
// Good
if (count >= limit) {
    total += values[i] * 2;
    log_value(total, i);
}
for (i in {0:10}) { ... }

// Bad
if(count>=limit){
    total+=values[ i ]*2;
    log_value (total,i);
}
for (i in { 0 : 10 }) { ... }
```

### One Statement per Line

```zacklang
// Good
let a = 1;
let b = 2;

// Bad
let a = 1; let b = 2;
```

### Blank Lines

Separate functions and struct definitions with one blank line. Inside a function, use a blank line to separate steps, not after every statement.

### switch Layout

Indent `case` and `finally` one level inside the `switch`, and indent each branch's statements one more level.

```zacklang
switch (command) {
    case "start", "go":
        begin();
    case "stop":
        halt();
    finally:
        report_unknown(command);
}
```

## Naming

| Item | Convention | Example |
|------|------------|---------|
| Functions | `snake_case`, verb or verb phrase | `calculate_average` |
| Variables and parameters | `snake_case`, noun | `user_age` |
| Struct fields | `snake_case` | `balance` |
| Struct types | `PascalCase` | `BankAccount` |
| Constants (`let const`) | `SCREAMING_SNAKE_CASE` | `MAX_ATTEMPTS` |
| `comptime` functions | same as functions | `factorial` |

- Names should say what something is for. A loop counter may be `i`, `j` or `k`; most other names should be words.
- Name `bool` values and functions that return `bool` as questions: `is_empty`, `has_key`, `can_move`.
- Do not reuse the names of built-ins (`print`, `prompt`, `panic`, `len`) for your own variables, even though a local variable is allowed to shadow them.
- Avoid shadowing a variable from an enclosing block. The language allows it, but it makes code harder to read.

```zacklang
// Good
let const MAX_ATTEMPTS = 3;
let is_valid = attempts < MAX_ATTEMPTS;

// Bad
let const maxAttempts = 3;
let v = attempts < maxAttempts;
```

## Types and Literals

### When to Write a Type

ZackLang infers the type of a variable from its initializer. Use that when the type is obvious, and write the type when it adds information.

- **Omit** the type when the initializer makes it obvious: a literal, a struct literal, or a conversion with `as`.
- **Write** the type when you want something other than the default (`i64` instead of `i32`, `f32` instead of `f64`).
- **Write** the type when the initializer is a function call whose result type is not obvious from its name.
- **Always write** the type when a variable is declared without an initializer. The language requires it.
- Parameter types, return types and struct field types are always written; the language requires it.

```zacklang
// Good
let count = 0;
let origin = Point { x: 0, y: 0 };
let total: i64 = 0;                  // wider than the default on purpose
let result: Report = build_report(data);
let label: string;                   // required: no initializer

// Bad
let count: i32 = 0;                  // the type adds nothing
let result = build(data);            // reader cannot tell the type
```

### Numeric Literals

- Use `_` to group long numbers: `1_000_000`.
- Use hexadecimal or binary for bit patterns and masks: `0xFF`, `0b1010`.
- Write decimal numbers with digits on both sides of the dot, such as `0.5`, so they read clearly (`.5` is valid but less obvious).
- Remember that an unadorned integer literal defaults to `i32`. If a value may not fit in 32 bits, give the variable an `i64` type.

## Operators and Conversions

- Add parentheses when mixing `and`, `or` and `xor`, even where precedence would make them unnecessary.
- Add parentheses around `**` with a negative base: write `(-2) ** 2`; `-2 ** 2` is `-4`.
- Never chain comparisons (`a < b < c` is an error). Write `a < b and b < c`.
- Prefer compound assignment (`total += x`) when updating a variable from its own value.
- Prefer `not is_valid` to comparing with `false`.
- Conversions must be explicit. Convert once, close to where the value is needed, and keep the number of `as` casts small. Prefer choosing the right type from the start.

```zacklang
// Good
if ((is_member and has_ticket) or is_staff) { ... }
let average = total as f64 / count as f64;

// Bad
if (is_member and has_ticket or is_staff) { ... }
if (is_valid == true) { ... }
let average = (total / count) as f64;     // divides as integers first, then converts
```

## Control Flow

- **Return early** for invalid or trivial cases. It keeps the main logic at the lowest indentation level.
- Prefer `for` over `while` when the number of iterations is known.
- Prefer `for (x in array)` over indexing when you only need the elements.
- Use `switch` when choosing between three or more values of one variable, and always add a `finally` branch that handles the unexpected case.
- Use `elif` instead of nesting an `if` inside an `else`.
- Avoid nesting deeper than three levels. Extract a function instead.
- Do not leave empty `case` branches. Several values for one body are written `case 1, 2:`.

```zacklang
// Good
fn process(data: i32[], count: i32): bool {
    if (count <= 0) {
        return false;
    }
    // main logic here
    return true;
}

// Bad: the main logic is buried
fn process(data: i32[], count: i32): bool {
    if (count > 0) {
        // main logic here
        return true;
    } else {
        return false;
    }
}
```

## Functions

- Keep functions short and focused on one task.
- Prefer at most four parameters. When several parameters belong together, group them in a struct.
- Order parameters logically: the main subject first, options last.
- Prefer functions that compute and return a result over functions that print. Keep input and output in `main` or in small helper functions so the logic is easy to test.
- Mark a function `comptime` only when it is a pure computation that is useful at compile time, such as filling constants or array sizes.
- Parameters cannot be modified. To work on a modified version, copy it into a local variable first.

```zacklang
// Good: computes and returns
fn add_numbers(a: i32, b: i32): i32 {
    return a + b;
}

// Harder to test: mixes the calculation with output
fn add_and_print(a: i32, b: i32): void {
    print(f"{a} + {b} = {a + b}");
}
```

## Structs and Arrays

- Use `let const` for values that never change, including arrays and structs.
- Structs and fixed-size arrays are copied on assignment and when passed to functions. For large values, keep that cost in mind in hot loops.
- Put one field or one struct literal field per line when they do not fit on one line, and end struct declarations with a trailing comma.
- Prefer a struct to parallel arrays (`names[]` and `scores[]` kept in sync).
- Array and struct literals that span several lines list one element per line. Array literals do not take a trailing comma.

```zacklang
struct Student {
    name: string,
    score: i32,
}

let const PASSING_SCORE = 60;

fn main(): void {
    let students = [
        Student { name: "Ada", score: 90 },
        Student { name: "Alan", score: 55 }
    ];
    for (s in students) {
        if (s.score >= PASSING_SCORE) {
            print(f"{s.name} passed");
        }
    }
}
```

## Comments

- Explain **why**, not **what**. The code already says what it does.
- Write a one- or two-line comment above each function that is not obviously self-explanatory, saying what it returns and any assumptions it makes.
- Start a comment with `//` followed by a space, and write complete sentences.
- Use `/* ... */` for a long explanation at the top of a file or to temporarily disable a block of code. Remember that block comments do not nest.
- Delete commented-out code instead of keeping it.

```zacklang
// Good
// Returns the index of the target, or -1 when it is not present.
// The array must be sorted in ascending order.
fn binary_search(values: i32[], target: i32): i32 {
    // ...
}

// Bad
// sets x to 5
let x = 5;
```

## File Organization

- Order the contents of a file: constants, struct definitions, helper functions, then `main` at the end. Functions can be used before they are defined, so this order is for readability only.
- Group related functions together.
- Declare constants at the top level with `let const` and give them a descriptive `SCREAMING_SNAKE_CASE` name instead of repeating a number.

```zacklang
let const MAX_SIZE = 100;
let const DEFAULT_RATE = 0.5;

struct Config {
    size: i32,
    rate: f64,
}

fn default_config(): Config {
    return Config { size: MAX_SIZE, rate: DEFAULT_RATE };
}

fn main(): void {
    let config = default_config();
    print(config.size);
}
```

## Errors and Safety

ZackLang stops the program with a panic on a violated rule (out-of-range index, integer division by zero, overflow in debug builds, `panic(...)`) and a panic cannot be caught. Write code so that panics mean bugs, not normal situations.

- **Check preconditions early.** Verify inputs at the start of a function. For a condition that can only be violated by a bug in the caller, call `panic` with a message that names the function and the problem.
- **Do not hide failures.** Printing a message and returning a made-up value (such as `0`) makes the bug harder to find. Use `panic`, or a return value that the caller must examine.
- **Check before dividing.** Integer division by zero panics, and decimal division by zero silently gives infinity or `NaN`; check the divisor yourself when it can be zero.
- **Check indexes that come from outside the program** before using them.
- **Do not rely on overflow behaviour.** Integers panic on overflow in debug builds and wrap in release builds, so the same code can behave differently. Use a wider type (`i64`) when values can get large.
- **Prefer constants** over repeating literal values, and prefer `let const` whenever a value does not change.

```zacklang
// Good
fn divide(a: i32, b: i32): i32 {
    if (b == 0) {
        panic("divide: the divisor is zero");
    }
    return a / b;
}

// Bad: hides the problem
fn divide(a: i32, b: i32): i32 {
    if (b == 0) {
        print("Error: division by zero");
        return 0;
    }
    return a / b;
}
```

### Making Code Testable

- Keep functions free of hidden state and side effects where possible.
- Make a function's inputs and outputs explicit through its parameters and return value.
- Check an invariant by writing an `if` that calls `panic` when it is broken:

```zacklang
if (not (low <= high)) {
    panic("range: low must not be greater than high");
}
```
