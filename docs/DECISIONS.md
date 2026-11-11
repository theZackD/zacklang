# ZackLang Decision Log

Settled decisions for the restart of the compiler. Anything not listed here is still open (see [Open decisions](#open-decisions)). The previous implementation is archived on the `legacy` branch (tag `legacy-v0`), including the original ZIR design docs (`docs/zir_*.md` and `docs/zacklang_to_zir.md` on that branch).

## Settled

| # | Area | Decision | Notes |
|---|------|----------|-------|
| D1 | Compiler language | C++17 or later | Front-end (lexer, parser, semantic analysis, typed AST) is plain C++ using the standard library. |
| D2 | Backend | LLVM, pinned to **LLVM 20.x** | Upgrades are planned tasks, never implicit. |
| D3 | LLVM access | LLVM **C API** (`llvm-c/*`) behind a single `Backend` class | C API is "best effort" stable (see LLVM Developer Policy), not guaranteed; instruction building and debug info are the least stable parts. Keeps the door open for self-hosting through an FFI. Phase 0 spike validates this; if too awkward, only `Backend` changes. |
| D4 | Intermediate representation | **No ZIR for now.** Pipeline: source, lexer, parser, semantic analysis, typed AST, LLVM IR | Comptime is evaluated by an interpreter over the typed AST. Lowering of f-strings, `for` ranges and `switch` happens in codegen. Codegen sits behind one narrow interface taking the typed AST so an IR can be inserted later. |
| D5 | IR revisit gate | Before the memory-management phase, decide whether a CFG IR is needed | Needed if ownership/borrow checking or drop elaboration is chosen. Not needed for GC, reference counting, or manual memory with checks. |
| D6 | Optimization | Delegated to LLVM (`-O2`/`-O3`) | No custom optimization passes. |
| D7 | Variable types | Annotation optional: `let x = 42;` infers from the initializer, `let x: i32 = 42;` is allowed | Annotation required when there is no initializer. Function parameter and return types are always explicit. |
| D8 | Numeric conversions | Literals adapt to the expected type. No other implicit conversions. Explicit casts required | `let x: i64 = 1;` and `let y: f32 = 1.5;` compile. `i32` to `i64` needs a cast. A literal that does not fit its target type is a compile error. |
| D9 | `switch` | No fall-through. Each `case` runs only its own body. `finally` is the default branch | |
| D10 | Language scope | The full set of features in the docs, aiming at a complete, usable language | Delivered phase by phase (see implementation plan). |
| D11 | Self-hosting | Later. First a working compiler, then iterate | Features needed for self-hosting (pointers/references, modules, FFI, generics, etc.) are specified when we get there. |
| D12 | Performance goals | Runtime speed comparable to C/Rust/Go | No GC pauses by default, value-type structs and arrays, no hidden allocations, defined integer overflow behavior that LLVM can still optimize, cheap bounds checks. Benchmarks against C and Rust at each milestone. |
| D13 | Process | Docs first, then phased implementation tracked in `IMPLEMENTATION_PLAN.md` | Each sub-phase has an acceptance criterion and a test. Tests check behavior (compile and run), not LLVM IR text. |
| D14 | Assignment | Statement only, not an expression. Compound assignment `+=`, `-=`, `*=`, `/=`, `%=` is supported | `a = b = c` is not valid. |
| D15 | Explicit casts | `x as i64` | Required for every numeric conversion except literal adaptation (D8). |
| D16 | Comments | `// line` and `/* block */` (block comments do not nest) | `//` is never an operator. |
| D17 | Division | `int / int` is integer division truncating toward zero. Division or modulo by zero always panics | Applies in all build modes. |
| D18 | Top level and entry point | Only declarations (`fn`, `struct`, `let const`) at top level. Entry point is `fn main(): void` or `fn main(): i32` (exit code) | No script-style top-level statements. |
| D19 | Integer overflow | Traps (panics) in debug builds, wraps (two's complement) in release builds | Explicit wrapping/checked operations may be added to the standard library later. |
| D20 | Power operator | `**` works on all numeric types and is right-associative. It binds tighter than unary minus on its left, so `-2 ** 2` is `-(2 ** 2)`. `integer ** negative integer` is an error (compile time if constant, otherwise a panic) | Result type is the type of the operands (both must have the same type after literal adaptation). |
| D21 | Arrays | Fixed-size `i32[5]` (compile-time length) and dynamic `i32[]` (growable). Both are zero-indexed and bounds-checked | Dynamic arrays need heap allocation, so their implementation waits for the memory model (gate D5). The spec defines them now. |
| D22 | `char` and `string` | `char` is a Unicode scalar value written `'a'`. `string` is immutable UTF-8. Escapes: `\n \t \r \\ \" \' \0 \u{XXXX}` | |
| D23 | `for` loop | `for (i in {start:end})`: start-inclusive, end-exclusive, step 1, upward. `for (x in array)` iterates over array elements | An empty range (`start >= end`) runs zero times. |
| D24 | Numeric literals | Decimal, hex `0x`, binary `0b`, `_` separators, floats with exponent (`1.5e3`). A negative number is unary minus applied to a literal | Literal adaptation rules from D8 apply. |
| D25 | Structs | Value types (copied on assignment). Declared `struct Point { x: f64, y: f64 }`, created `Point { x: 1.0, y: 2.0 }`, accessed `p.x`. Fields are assignable when the variable is not const | |
| D26 | `switch` details | Subject types: integers, `char`, `bool`, `string`. Several values per case (`case 1, 2:`). Duplicate case values are an error. `break` is not needed and cannot be used to leave a switch | Builds on D9 (no fall-through, `finally` is the default branch). |
| D27 | Comptime | Pure functions only. Allowed: arithmetic, comparisons, `if`/`while`/`for`, local variables, calls to other comptime functions, fixed-size arrays and structs. Execution is limited by a step budget and a recursion depth; exceeding either is a compile error. Top-level `let const` must be comptime-evaluable | Evaluated by an interpreter over the typed AST (D4). |
| D28 | Conditions and equality | Conditions must be `bool` (no truthiness). `and`/`or` short-circuit. `==` and `!=` work on two values of the same type; strings and arrays compare by content | |
| D29 | Constants and initialization | `let const` is immutable after initialization. `let` without an initializer requires a type annotation, and the compiler checks definite assignment before use. No implicit default values | |
| D30 | Functions | Callable before their definition. Recursion and mutual recursion allowed. No overloading, no default arguments. Parameters are immutable inside the function. No first-class functions yet | |
| D31 | Panics | A panic prints a message with the source line to stderr and exits with code 101. Panics cannot be caught in v0.1. Triggers: out-of-bounds index, division or modulo by zero, integer overflow in debug builds, `integer ** negative`, explicit `panic(msg)` | |
| D32 | Scoping | Block scoping. Shadowing an outer variable in an inner block is allowed. Redeclaring a name in the same scope is an error. Top-level names are visible everywhere in the file | |
| D33 | `print` and `prompt` | Built-in functions, not keywords. `print` accepts any printable value. `prompt(message): string` returns the line read. Numeric conversions use `as`. String conversions use library functions (`parse_i32`, `to_string`, ...) | |
| D34 | Float literal forms | A float literal may start with `.` (`.5` is `0.5`). A trailing dot (`5.`) is invalid | `.` followed by a digit starts a float, so field access `p.x` is not affected. Amends D24. |
| D35 | Arrays of arrays | Allowed. `T[A][B]` is an array of `A` elements of type `T[B]`, so `a[i][j]` has `i < A` and `j < B` (row-major) | Types are recursive. Value or shared semantics of nested dynamic arrays follow the memory model (gate D5). Amends D21. |
| D36 | Grammar-level rules | (1) String and char literals cannot span lines. (2) F-string interpolations may contain string literals and nested f-strings. (3) Trailing commas are allowed in struct declarations and struct literals only. (4) A bare block `{ ... }` is a valid statement. (5) `case` and `finally` bodies are brace-less statement lists, each its own scope. (6) There is no unary `+`. | Listed in `grammar.ebnf.md`. |
| D37 | Parameter scope | Function parameters live in the function body's outermost scope. Declaring a variable with a parameter's name at the top level of the body is an error; an inner block may shadow it | Consistent with D32. |
| D38 | Array literals and fixed/dynamic arrays | An array literal takes the expected type when one exists: `T[N]` (length must be exactly `N`) or `T[]`. Without an expected type it is the fixed array `T[N]`. There is no implicit conversion between a fixed array value and a dynamic array type; explicit conversion and growth operations (`push` and similar) belong to the standard library, which is not yet specified | Dynamic array semantics finish with the memory model (gate D5). |
| D39 | Semantic details | (1) Operands and call arguments are evaluated left to right. (2) Definite assignment and "all paths return" are structural analyses: an `if` chain counts only with an `else`, a `switch` only with a `finally`, loop bodies never count, and loops are never assumed to return. (3) `main` must exist exactly once, takes no parameters, returns `void` or `i32`. (4) Built-in names (`print`, `prompt`, `panic`, `len`) live at top level: declaring a top-level item with one of those names is an error; locals may shadow them. (5) Top-level constants may depend on each other in any order but not cyclically. | Specified in `language_reference.md`. |
| D40 | Compiler pipeline | source, lexer, parser, AST, semantic analysis, checked AST, codegen (LLVM IR via `Backend`), object file, link. Comptime interpreter runs over the checked AST and is called on demand by semantic analysis | Details in `compiler_architecture.md`. |
| D41 | AST design | One AST, annotated in place by semantic analysis (types and resolved names attached). Codegen consumes a `CheckedProgram` | Keeps the narrow codegen interface from D4 without duplicating node definitions. |
| D42 | Diagnostics | Collect multiple errors with source spans, caret output and error codes. Stop after a limit (default 20) | No exit-on-first-error, no `exit()` inside the compiler library. |
| D43 | Allocation before the memory model | The early runtime uses a never-freeing allocator for features that allocate (f-strings stored in variables, `prompt`, dynamic arrays). Replaced when the memory model is decided at gate D5 | Programs may leak memory until then. |
| D44 | Names | Source file extension `.zl`, compiler command `zackc` | |
| D45 | Platforms | macOS, Linux and Windows from the start (arm64 and x86-64 where each OS supports them) | Needs CI on all three systems and a check that LLVM 20 (headers, libraries and tools) is available on each. Development machine is macOS, so Windows and Linux are verified through CI or a VM. |
| D46 | Runtime library | Written in C, compiled to a static library (`libzackrt`) that every program links. It starts minimal (I/O, panic, allocation, string helpers). The standard library moves into ZackLang over time, leaving a small C core for what only the OS can do | Helps self-hosting later. Calling convention is plain C (`zack_*` functions). |
| D47 | Build system | CMake | Works on macOS, Linux and Windows, and LLVM provides its files in CMake form. Chosen by the project owner in Phase 0, step 1. |
| D48 | Unit test framework | Catch2 v3, fetched by CMake at a pinned version | Popular and cross-platform. Needs network access when first configuring the build. Delegated to the assistant by the project owner in Phase 0, step 2; can be changed. |
| D49 | Whole-program test runner | Golden and end-to-end tests run through a small C++ program, `tests/tools/expect_runner`, that checks exit code, standard output and standard error (no Python anywhere in the project). It reuses the driver's process launcher. Everything is registered with CTest. `ZACK_UPDATE_GOLDEN=1` rewrites golden files | Chosen by the project owner in Phase 0 review ("C++ referee"). The referee has its own tests proving it fails when expectations are wrong. |
| D50 | LLVM discovery and sanitizers | CMake finds LLVM without a version argument (LLVM's own version file only accepts an exact major.minor) and enforces major version 20 itself. Sanitizer builds apply only to the compiler, never to `libzackrt`, because user programs are not linked against a sanitizer runtime | Found in Phase 0; see `compiler_architecture.md`, Build and Toolchain. |
| D51 | Linking | `zackc` links programs by calling the system C compiler driver (`cc` on Unix-like systems, `clang` on Windows; override with the `ZACK_CC` environment variable). Programs are launched with argument arrays, never through a shell | Chosen by the project owner in Phase 0 review: keep it for now and revisit close to a release. Users need a C compiler installed. All of it lives in `src/driver/linker.cpp`. |

## Open decisions

To be resolved while fixing the remaining docs, one doc at a time.

- Memory model (GC, reference counting, ownership/borrowing, manual with checks): decide at gate D5. Also decides whether dynamic arrays are copied or shared on assignment.
- F-string format specifiers (`{x:5}`) and the text form of floats; printing of arrays and structs. Interpolation syntax itself is settled (D36).
- Standard library: string operations and concatenation, growing dynamic arrays (`push` and similar), text/number parsing and formatting, math functions.
- Creating an array of a given size with a repeated initial value (for example a zero-filled `i32[N]` where `N` is a constant). Today an array can only be created from an explicit literal, which is impractical for large sizes. Needs a fill syntax or a standard-library function.
- Whether `+` joins strings or a separate operator is used (depends on the memory model, since the result is allocated).
- Bitwise operators and shifts.
- Error handling beyond panics (for example a `Result` type).
- Module and import system.
- Generics.
- First-class functions and closures.

## Rejected

| Option | Why |
|--------|-----|
| Full ZIR with own optimization passes | Duplicates LLVM; the legacy implementation of these passes had many correctness problems. |
| Plain C for the whole compiler | Manual memory management and no standard containers; the legacy C front-end had many leaks and unchecked allocations. |
| LLVM C++ API | No stability policy and not callable from a self-hosted compiler. |
