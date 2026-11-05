# ZackLang Language Reference

The precise definition of the ZackLang language. If you are new to ZackLang, start with the [Language Guide](language_guide.md), which teaches the language by example, and come back here when you need an exact rule.

Related documents:

- [grammar.ebnf.md](grammar.ebnf.md): the formal grammar (what is syntactically valid).
- [DECISIONS.md](DECISIONS.md): why the language is the way it is. Rules below carry a tag such as (D17) pointing at the matching decision.

> **Status:** the compiler is being rebuilt from this specification and does not exist yet. Items marked *not yet specified* are deliberately left open.

## How to Read This Document

- Code blocks in `zacklang` show valid or invalid code, as the comments say.
- **Compile-time error** means the compiler rejects the program. **Panic** means the program stops while running (see [Runtime Errors](#runtime-errors)).
- "Must" describes a rule the compiler enforces.
- Snippets of statements are meant to appear inside a function body.

## Contents

1. [Program Structure](#program-structure)
2. [Lexical Structure](#lexical-structure)
3. [Types](#types)
4. [Conversions](#conversions)
5. [Expressions](#expressions)
6. [Statements](#statements)
7. [Scope and Names](#scope-and-names)
8. [Functions](#functions)
9. [Structs](#structs)
10. [Compile-time Evaluation](#compile-time-evaluation)
11. [Runtime Errors](#runtime-errors)
12. [Built-in Functions](#built-in-functions)
13. [Compile-time Errors at a Glance](#compile-time-errors-at-a-glance)

## Program Structure

A source file is a sequence of **top-level declarations** (D18). Only three kinds exist:

| Declaration | Example |
|-------------|---------|
| function | `fn add(a: i32, b: i32): i32 { return a + b; }` |
| struct | `struct Point { x: f64, y: f64 }` |
| constant | `let const LIMIT = 100;` |

Statements are not allowed at top level.

- The program starts at `main`. Exactly one `main` must exist, it takes no parameters, and its return type is `void` or `i32`. An `i32` result is the process exit code; a `void` main exits with code 0.
- Declarations can appear in any order. A top-level constant may use other top-level constants and `comptime` functions; a cycle of constants is a compile-time error.

```zacklang
let const GREETING = "Hello";

fn main(): void {
    print(GREETING);
}
```

## Lexical Structure

### Source Text and Whitespace

Source is UTF-8. Spaces, tabs and line breaks separate tokens and are otherwise ignored.

### Comments (D16)

```zacklang
// line comment

/* block comment, may span lines, does not nest */
```

`//` is never an operator.

### Identifiers

Letters (`a-z`, `A-Z`), digits and `_`, not starting with a digit. Case-sensitive. A keyword cannot be used as an identifier.

### Keywords

```
let       const     fn        return    struct
if        elif      else      switch    case
finally   while     for       in        break
continue  comptime  as        true      false
and       or        not       xor
i32       i64       f32       f64       bool
char      string    void
```

Reserved for the future (cannot be used as identifiers): `import`, `enum`, `null`.

`print`, `prompt`, `panic` and `len` are not keywords; they are built-in functions (D33).

### Operators and Punctuation

| Category | Tokens |
|----------|--------|
| Arithmetic | `+` `-` `*` `/` `%` `**` |
| Comparison | `==` `!=` `<` `>` `<=` `>=` |
| Logical | `and` `or` `not` `xor` |
| Assignment | `=` `+=` `-=` `*=` `/=` `%=` |
| Conversion | `as` |
| Punctuation | `(` `)` `{` `}` `[` `]` `,` `;` `:` `.` |

### Number Literals (D24, D34)

```zacklang
42          1_000_000       // decimal; _ may separate digits
0xFF        0b1010          // hexadecimal, binary
3.14        0.001           // decimal numbers
.5                          // 0.5: a leading dot is allowed
1.5e3       2e-4            // exponent form
```

- `5.` (trailing dot) is not valid; write `5.0`.
- A literal has no sign. `-456` is unary minus applied to `456`. A negated literal is range-checked as a whole, so `-2147483648` is a valid `i32`.
- Letters directly after a number (`12abc`) are an error.

### Boolean Literals

`true` and `false`.

### Character and String Literals (D22, D36)

```zacklang
'a'   '\n'   '\u{1F600}'           // char: one Unicode scalar value
"Hello, World!"                    // string
```

- Characters and strings cannot span lines. Use `\n`.
- Escapes:

| Escape | Meaning |
|--------|---------|
| `\n` `\t` `\r` | line feed, tab, carriage return |
| `\\` | backslash |
| `\"` | double quote |
| `\'` | single quote |
| `\0` | null character |
| `\u{X...}` | Unicode scalar value, 1 to 6 hex digits |

### F-strings (D36)

```zacklang
f"Value: {x}, sum: {a + b}, braces: {{ }}"
```

- `{expression}` is evaluated and converted to text; `{{` and `}}` produce `{` and `}`.
- An interpolation may contain any expression, including string literals and nested f-strings.
- The result has type `string`. Format specifiers are not supported.
- Printable values are those of the primitive types (D33). The text form of floats is not yet specified, and arrays and structs are not yet printable.

## Types

### Primitive Types

| Type | Description |
|------|-------------|
| `i32` | 32-bit signed integer |
| `i64` | 64-bit signed integer |
| `f32` | 32-bit IEEE 754 floating point |
| `f64` | 64-bit IEEE 754 floating point |
| `bool` | `true` or `false` |
| `char` | one Unicode scalar value |
| `string` | immutable UTF-8 text |
| `void` | no value; only valid as a function's return type |

### Array Types (D21, D35, D38)

| Type | Meaning |
|------|---------|
| `T[N]` | fixed-size array of exactly `N` elements; `N` is a compile-time constant integer |
| `T[]` | dynamic array whose length can change at run time |

- Indexing starts at 0 and is bounds-checked: an out-of-range index panics.
- Fixed-size arrays are value types: assignment and argument passing copy all elements.
- Arrays of arrays are allowed. Suffixes read outermost first: `T[A][B]` is an array of `A` elements, each of type `T[B]`, so `a[i][j]` has `i < A` and `j < B`. `i32[2][3]` is 2 rows of 3 columns, stored row by row.
- The element type cannot be `void`.
- Dynamic arrays: how they are copied or shared on assignment and how they grow is decided with the memory model and the standard library (not yet specified). Parameters are immutable, so passing one to a function is observably the same either way.

```zacklang
let a: i32[3] = [1, 2, 3];
let g: i32[2][3] = [[1, 2, 3], [4, 5, 6]];
let cell = g[1][2];                    // 6
g[0][1] = 20;
let v: i32[] = [];
```

### Struct Types

Declared with `struct` (see [Structs](#structs)). Struct types are nominal: two struct types are the same only if they have the same name.

### Type Inference (D7)

- A variable declaration with an initializer may omit its type: `let x = 42;`.
- A declaration without an initializer must state its type.
- Parameter types, return types and struct field types are always explicit.
- With no other information: an integer literal is `i32`, a decimal literal is `f64`, `true`/`false` are `bool`, `"..."` is `string`, `'...'` is `char`, an array literal `[e1, ..., eN]` is the fixed array `T[N]` where `T` is the element type.

### Literal Adaptation (D8, D38)

A literal takes the type its context expects, if the value fits:

- An integer literal adapts to any integer type (`i32`, `i64`). It does **not** adapt to `f32` or `f64`: write `1.0`.
- A decimal literal adapts to `f32` or `f64`. It never adapts to an integer type.
- In a binary operation, a literal operand adapts to the type of the other operand. When both operands are literals the context decides, defaulting to `i32` and `f64`.
- A literal that does not fit its target type is a compile-time error. This includes results of constant expressions.
- An array literal adapts to the expected array type, fixed `T[N]` (it must have exactly `N` elements) or dynamic `T[]`, and its elements adapt to `T`. An empty literal requires an expected type.

```zacklang
let a: i64 = 1;             // ok
let b: f32 = 1.5;           // ok
let c: i32 = 3_000_000_000; // error: does not fit
let d: f64 = 1;             // error: write 1.0
let e: i32[] = [];          // ok: expected type given
let f = [];                 // error: no expected type
```

## Conversions

Apart from literal adaptation there are **no implicit conversions**. Every other conversion is written with `as` (D15):

```zacklang
let n = 5;
let wide: i64 = n;          // error
let wide2: i64 = n as i64;  // ok
```

| From | To | Result |
|------|----|--------|
| integer | integer | widening sign-extends; narrowing keeps the low bits |
| integer | float | nearest representable value |
| float | integer | truncates toward zero; out-of-range values saturate; `NaN` becomes 0 |
| float | float | nearest representable value |
| `bool` | integer | `false` is 0, `true` is 1 |
| `char` | `i32`, `i64` | the Unicode scalar value |
| `i32` | `char` | the character; panics if not a valid Unicode scalar value |

Any other conversion (for example `int as bool`, or anything involving `string`) is a compile-time error. Converting between text and numbers will be provided by the standard library (not yet specified).

## Expressions

### Evaluation Order

Operands of an operator and arguments of a call are evaluated left to right (D39). The right operand of `and` and `or` is evaluated only when needed.

### Operator Precedence

From tightest to loosest. Binary operators are left-associative unless stated.

| Level | Operators | Notes |
|-------|-----------|-------|
| 1 | call `f(x)`, index `a[i]`, field `p.x` | postfix |
| 2 | `**` | right-associative; the right operand may have a leading `-` (`2 ** -1`) |
| 3 | unary `-`, `not` | prefix |
| 4 | `as` | `-x as i64` is `(-x) as i64` |
| 5 | `*` `/` `%` | |
| 6 | `+` `-` | |
| 7 | `<` `<=` `>` `>=` | cannot be chained |
| 8 | `==` `!=` | cannot be chained |
| 9 | `and` | |
| 10 | `xor` | |
| 11 | `or` | |

`**` binds tighter than unary minus (D20): `-2 ** 2` is `-(2 ** 2)`, which is `-4`. There is no unary `+` (D36). `a < b < c` and `a == b == c` are syntax errors.

### Arithmetic

`+ - * / % **` need both operands of the same numeric type, after literal adaptation. The result has that type. `+` does not join strings (string operations are not yet specified).

- `/` on integers truncates toward zero. `/` or `%` with a zero divisor panics (D17).
- `%` on integers takes the sign of the left operand (`-7 % 2` is `-1`). `%` on floats is the floating-point remainder.
- `**` works on all numeric types. `integer ** negative integer` is a compile-time error when both operands are constants and a panic otherwise (D20).
- Integer overflow panics in debug builds and wraps in two's complement in release builds (D19). Overflow in a compile-time constant expression is a compile-time error.
- Floating-point arithmetic follows IEEE 754 and never panics (division by zero gives infinity or `NaN`).
- Unary `-` applies to numeric types.

### Comparison

`== != < <= > >=` need both operands of the same type and give a `bool`.

- `==` and `!=` work on every type. Strings and arrays compare by content and structs field by field (D28). Floats follow IEEE 754 (`NaN` is not equal to anything).
- `<` `<=` `>` `>=` work on numbers and `char`. Strings compare lexicographically by Unicode scalar value. `bool`, arrays and structs are not ordered.

### Logical Operators

`and`, `or`, `xor`, `not` need `bool` operands and give a `bool`. `and` and `or` short-circuit (D28). `xor` always evaluates both sides.

### Function Calls

```zacklang
add(2, 3)
```

Only a function name can be called. The argument count and types must match the declaration, with literal adaptation applied to each argument. Arguments are passed by value (D30).

### Indexing

```zacklang
values[i]
grid[row][col]
```

The index must be an integer type. An out-of-range index panics (a constant index that is provably out of range for a fixed-size array may be reported at compile time).

### Array Literals

```zacklang
[1, 2, 3]
[[1, 2], [3, 4]]
```

All elements must have the same type after adaptation. See [Literal Adaptation](#literal-adaptation-d8-d38).

### Struct Literals and Field Access (D25)

```zacklang
Point { x: 1.0, y: 2.0 }
p.x
```

Every field must be given exactly once, in any order. A trailing comma is allowed in struct declarations and struct literals but not in parameter, argument or array-literal lists (D36).

### Conversions

```zacklang
value as Type
```

See [Conversions](#conversions). `as` can be chained: `x as i64 as f64`.

## Statements

Simple statements end with `;`: variable declaration, assignment, call, `return`, `break`, `continue`. Blocks and control-flow statements do not take a `;`. A bare block `{ ... }` is a statement of its own and opens a new scope (D36).

### Variable Declaration (D7, D29)

```zacklang
let x = 1;                  // variable, type inferred
let y: f64 = 2.0;           // variable, explicit type
let const Z = 3;            // constant
let w: i32;                 // declared; assigned later
```

- `let const` requires an initializer and can never be assigned afterwards. This is deep: elements of a constant array and fields of a constant struct cannot change either.
- A declaration without an initializer needs a type annotation.
- A variable must be **definitely assigned** before it is read: on every path leading to the read, some assignment has happened. The compiler checks this structurally:
  - after an `if` / `elif` / `else` chain it is assigned only if there is an `else` and every branch assigns it;
  - after a `switch` it is assigned only if there is a `finally` and every branch assigns it;
  - assignments inside `while` or `for` bodies do not count after the loop (the body might not run).
- There are no implicit default values (D29).

### Assignment (D14)

```zacklang
target = expression;
target += expression;       // also -=, *=, /=, %=
```

- A target is a variable, an array element `a[i]`, or a struct field `p.x`, possibly nested (`g[1][2]`, `a.b.c`).
- The variable at the root of the target must not be `const` and must not be a parameter or a loop variable.
- `a op= b` means `a = a op b` with the target evaluated once.
- The types must match after literal adaptation.
- Assignment is a statement, not an expression (`a = b = c` is invalid).

### Call Statement

```zacklang
greet("Ada");
```

Only function calls can be used as statements. The result of a non-`void` call is discarded.

### if / elif / else

```zacklang
if (condition) {
    // ...
} elif (condition) {
    // ...
} else {
    // ...
}
```

Parentheses and braces are required. Each condition must be a `bool` (D28). `elif` and `else` are optional.

### switch (D9, D26)

```zacklang
switch (value) {
    case 1, 2:
        // ...
    case 3:
        // ...
    finally:
        // ...
}
```

- The subject is an integer, `char`, `bool` or `string`.
- Case values must be constants of the subject's type (after literal adaptation) and must be distinct.
- Exactly one branch runs: the first `case` whose value equals the subject, otherwise `finally` if present, otherwise nothing. **There is no fall-through.**
- Each branch is a list of statements and a scope of its own. `finally` is optional and must come last.
- `break` and `continue` inside a `switch` refer to the enclosing loop and are errors if there is none.

### while

```zacklang
while (condition) {
    // ...
}
```

The condition is tested before each iteration and must be a `bool`.

### for (D23)

```zacklang
for (i in {start:end}) { ... }      // range
for (x in array) { ... }            // array elements
```

- Range: `i` takes `start`, `start + 1`, ..., `end - 1`. `start` and `end` must have the same integer type (literals adapt to the other side, defaulting to `i32`), which becomes the type of `i`. If `start >= end` the body does not run. Both bounds are evaluated once, before the loop.
- Array: `x` visits each element in order and holds a copy of it. The array is evaluated once.
- The loop variable is a new variable scoped to the loop body and cannot be assigned.

### return

```zacklang
return;
return expression;
```

The expression must match the function's return type (literal adaptation applies). A `void` function uses `return;` or reaches the end of its body.

### break and continue

`break;` leaves the innermost loop. `continue;` jumps to the next iteration of the innermost loop. Both are compile-time errors outside a loop. They cannot cross a function boundary.

## Scope and Names (D32, D37)

- A block `{ ... }` opens a scope. A variable is visible from its declaration to the end of its block.
- An inner block may declare a name that exists in an outer scope; the inner name hides the outer one (shadowing).
- Declaring a name twice in the same scope is a compile-time error.
- Function parameters belong to the outermost scope of the function body, so a parameter's name cannot be declared again at the top of the body. An inner block may shadow it.
- Top-level names (functions, structs, constants) are visible everywhere in the file and must be unique among themselves.
- Built-in function names are part of the top level: declaring a top-level item called `print`, `prompt`, `panic` or `len` is an error. A local variable may shadow them.

```zacklang
let x = 1;
{
    let x = 2;       // ok: shadows the outer x
    print(x);        // 2
}
print(x);            // 1
```

## Functions

```zacklang
[comptime] fn name(param1: type1, param2: type2): return_type {
    // body
}
```

- Parameter types and the return type are written explicitly. A function returning nothing is declared `: void`.
- Parameters are passed by value and are immutable inside the function (D30).
- A function can be called before its definition. Recursion and mutual recursion are allowed.
- No overloading, no default arguments. Functions are not first-class: they cannot be stored in variables or passed as arguments.
- **All paths return.** In a function with a non-`void` return type, every path must end in a `return`. The check is structural: a block returns if its last statement is a `return`, an `if` chain with an `else` in which every branch returns, or a `switch` with a `finally` in which every branch returns. Loops are never assumed to return (D39).

```zacklang
fn sign(n: i32): i32 {
    if (n < 0) {
        return -1;
    } elif (n > 0) {
        return 1;
    } else {
        return 0;
    }
}
```

## Structs (D25)

```zacklang
struct Point {
    x: f64,
    y: f64,
}

fn length_squared(p: Point): f64 {
    return p.x * p.x + p.y * p.y;
}

fn main(): void {
    let p = Point { x: 3.0, y: 4.0 };
    p.x = 6.0;                          // ok: p is not const
    print(length_squared(p) == 52.0);   // true
}
```

- Structs are value types: assignment and argument passing copy the value.
- Field types may be primitives, arrays or other structs. A struct cannot contain itself, directly or through other structs.
- Field names within a struct must be unique.
- `p.x = value;` is allowed when the root variable `p` is not `const`.

## Compile-time Evaluation (D27)

A function marked `comptime` can be run by the compiler.

```zacklang
comptime fn factorial(n: i32): i32 {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

let const FACT_5 = factorial(5);    // 120, computed while compiling
```

**Rules**

1. A `comptime` function must be pure: no input or output, no panics other than the checked errors in rule 6, and no access to state outside itself.
2. Allowed inside: arithmetic, comparisons, `if`, `while`, `for`, local variables, calls to other `comptime` functions, fixed-size arrays and structs.
3. Evaluation is limited by a step budget and a maximum recursion depth. Exceeding either is a compile-time error.
4. A top-level `let const` must be evaluable at compile time.
5. When every argument of a call is a compile-time constant, the call is evaluated by the compiler. Otherwise the function runs at run time like a normal function.
6. An error during compile-time evaluation (division by zero, overflow, out-of-range index) is a compile-time error.
7. Array sizes (`T[N]`), `case` values and constant initializers are evaluated at compile time.

## Runtime Errors (D31)

A **panic** prints a message with the source line to standard error and ends the program with exit code 101. Panics cannot be caught. They are caused by:

| Cause | Notes |
|-------|-------|
| out-of-range array index | all builds |
| integer `/` or `%` by zero | all builds |
| integer overflow | debug builds only; release builds wrap |
| `integer ** negative integer` | when not detected at compile time |
| `i32 as char` with an invalid value | all builds |
| `panic(message)` | explicit |

## Built-in Functions (D33)

| Function | Description |
|----------|-------------|
| `print(value)` | writes the value followed by a line break to standard output |
| `prompt(message): string` | writes `message` (no line break), reads one line from standard input and returns it without the line terminator |
| `panic(message)` | triggers a panic with the given `string` |
| `len(x): i64` | number of elements of an array, or of Unicode scalar values of a string |

Not yet specified: string operations (joining, slicing, searching), array operations (adding and removing elements), parsing and formatting of numbers, math functions, file I/O.

## Compile-time Errors at a Glance

A quick index of what the compiler rejects. Each item is explained above.

- Syntax errors; unterminated literals or block comments; reserved words used as names
- Unknown names; duplicate names in one scope; clashes with built-in names
- Type mismatches, including mixing number types without `as` and non-`bool` conditions
- Literals that do not fit their type; integer literals used as decimals; trailing-dot literals
- Missing type annotations where required (parameters, return types, struct fields, uninitialized variables)
- Assigning to `const` variables, parameters, loop variables, or elements/fields of constants
- Reading a variable that may not be assigned
- A non-`void` function with a path that does not return
- `break` or `continue` outside a loop
- Duplicate `case` values; non-constant `case` values
- Chained comparisons
- Missing, duplicate or invalid `main`
- Struct literals with missing, duplicate or unknown fields; recursive structs
- Array literal length mismatch; empty array literal without an expected type
- `comptime` violations, limits exceeded, or errors during compile-time evaluation
- Invalid conversions with `as`
