# ZackLang Compiler Architecture

How the ZackLang compiler (`zackc`) is structured: the stages, the data that flows between them, and the rules each stage must follow. Decisions behind it are in [DECISIONS.md](DECISIONS.md) (D1 to D46). The language itself is defined by the [Language Reference](language_reference.md) and the [Grammar](grammar.ebnf.md).

> **Status:** design only. Decisions D40 to D46 are settled. The details below those (data structures, naming, helper functions) are the proposed design and are meant to be refined in the implementation phases, with changes recorded in the decision log.

## Contents

1. [Goals](#goals)
2. [Pipeline Overview](#pipeline-overview)
3. [Repository Layout](#repository-layout)
4. [Common Infrastructure](#common-infrastructure)
5. [Lexer](#lexer)
6. [Parser](#parser)
7. [AST](#ast)
8. [Semantic Analysis](#semantic-analysis)
9. [Comptime Interpreter](#comptime-interpreter)
10. [Code Generation](#code-generation)
11. [Runtime Library](#runtime-library)
12. [Driver and Build Modes](#driver-and-build-modes)
13. [Build and Toolchain](#build-and-toolchain)
14. [Cross-cutting Rules](#cross-cutting-rules)
15. [Testing Strategy](#testing-strategy)
16. [Extension Points](#extension-points)

## Goals

1. **Correctness first.** The language rules in the reference are the specification; every compile-time error and runtime behavior it lists must be implemented and tested.
2. **Clear stage boundaries.** Each stage has one input, one output and one reason to change. A later stage never reaches back into an earlier one.
3. **Good diagnostics.** Multiple errors per run, exact source locations, and messages a beginner can act on (D42).
4. **Fast generated code** (D12): LLVM does the optimizing, and we emit IR that lets it (stack slots for locals, precise checks that LLVM can remove).
5. **Portable** (D45): macOS, Linux and Windows from the start.
6. **A path to self-hosting** (D11): keep the compiler's structure simple enough to be rewritten in ZackLang later, and keep LLVM behind one small class (D3).

## Pipeline Overview

```
 .zl source files
        │
        ▼
 ┌─────────────┐   tokens    ┌────────────┐    AST     ┌───────────────────┐
 │    Lexer    │ ──────────► │   Parser   │ ─────────► │ Semantic analysis │
 └─────────────┘             └────────────┘            └─────────┬─────────┘
                                                          ▲      │ CheckedProgram
                                       on-demand constant│      ▼
                                          evaluation ┌───┴──────────┐
                                                     │   Comptime   │
                                                     │ interpreter  │
                                                     └──────────────┘
                                                                │
        ┌───────────────────────────────────────────────────────┘
        ▼
 ┌─────────────┐  Backend calls  ┌──────────────────┐  LLVM IR   ┌───────────┐
 │  Lowering   │ ──────────────► │ Backend (LLVM C) │ ─────────► │ Object    │
 └─────────────┘                 └──────────────────┘            │ file      │
                                                                 └─────┬─────┘
                       libzackrt.a (C runtime) ────────────────────────┤
                                                                       ▼
                                                                ┌────────────┐
                                                                │   Linker   │ ─► executable
                                                                └────────────┘
```

| Stage | Input | Output | Reports |
|-------|-------|--------|---------|
| Lexer | source text | token list | lexical errors |
| Parser | tokens | AST | syntax errors |
| Semantic analysis | AST | `CheckedProgram` (annotated AST) | name, type, flow and constant errors |
| Comptime interpreter | checked expression or function | constant value | errors during compile-time evaluation |
| Lowering | `CheckedProgram` | calls on `Backend` | internal errors only |
| Backend | typed IR-building calls | LLVM module, object file | LLVM verification failures (internal errors) |
| Linker | object file and runtime library | executable | link errors |

Compilation stops after the first stage that reports an error (after finishing that stage, so all of its errors are shown). Later stages never run on invalid input.

## Repository Layout

```
src/
  common/      source manager, spans, diagnostics, arena, interner, UTF-8 helpers
  lexer/       tokens and lexer
  ast/         node definitions and the AST printer
  parser/      recursive-descent parser
  sema/        types, scopes, checker, flow analysis, CheckedProgram
  comptime/    compile-time interpreter and shared arithmetic helpers
  codegen/     lowering and the Backend class (the only code that includes llvm-c)
  driver/      command-line interface and build orchestration
runtime/       C runtime library (zackrt.h, zackrt.c)
tests/         unit, golden, end-to-end and differential tests
docs/          language and compiler documentation
```

Build system: CMake. Language standard: C++20 for the compiler (D1 requires C++17 or later). C11 for the runtime. All code lives in the namespace `zack`. Only the `codegen/` directory may include `llvm-c/*` headers (D3); everything else is independent of LLVM.

## Common Infrastructure

**SourceManager.** Owns the text of every source file and maps a byte offset to file, line and column. Lines are counted by `\n` (a preceding `\r` is ignored, so Windows line endings work). Columns count Unicode scalar values.

**Span.** `{file, begin, end}` byte offsets. Every token, AST node and diagnostic has one. Spans are never recomputed from line and column.

**Diagnostics.** A `Diagnostic` has a severity, an error code, a message, a primary span, and optional notes with their own spans.

- Error codes are stable and prefixed by stage: `L` lexer, `P` parser, `S` semantic analysis, `C` comptime, `G` code generation and internal errors. Example: `S0107: mismatched types`.
- The `DiagnosticEngine` collects diagnostics, counts errors, stops accepting new ones after a limit (default 20), and renders them with the source line and a caret (D42).
- The compiler library never calls `exit` and never prints directly. Only the driver renders diagnostics and chooses the exit code.

```
error[S0107]: mismatched types
  --> example.zl:4:18
   |
 4 |     let d: i64 = c;
   |                  ^ expected `i64`, found `i32`
   = help: convert explicitly with `c as i64`
```

**Arena.** A bump allocator that owns all AST nodes and types for one compilation. Nodes are never freed individually; the arena is released when compilation ends. This removes the leak and double-free problems of the legacy implementation.

**Interner.** Identifiers are interned to a small integer (`Symbol`), so comparisons are cheap and names are stored once.

**Determinism.** No container keyed or ordered by pointer address. Anything that is iterated (symbol tables, struct fields, diagnostics) has a defined order, so output is identical across runs and platforms.

## Lexer

Input: the text of one file. Output: a vector of tokens ending with an end-of-file token.

- **Token** = `{kind, span, value}`. Keywords, operators and punctuation are kinds. Identifiers carry an interned `Symbol`.
- **Escapes are decoded in the lexer.** A string or char token carries its decoded UTF-8 value, so no later stage sees backslashes.
- **Numbers:** the lexer records an integer literal's value as an unsigned 64-bit integer, and a decimal literal's value as a double, along with its written form (decimal, hex, binary). A value that does not fit in 64 bits is a lexical error. Range checks against `i32`, `i64` and negation (`-2147483648`) happen in semantic analysis, because they depend on the type the literal adapts to.
- **F-strings.** An f-string is one token holding a list of parts. A part is either literal text or an interpolation. An interpolation holds its own token list, produced by recursively lexing the characters between the braces (using the matching rule from the [grammar](grammar.ebnf.md)). The parser parses each interpolation with a sub-parser over those tokens. Because spans are absolute byte offsets, errors inside an interpolation point to the exact place in the file.
- **Comments** are discarded. An unterminated block comment, string, char literal or interpolation is a lexical error.
- **Recovery:** after an error the lexer skips to a safe point (the end of the line or the next token boundary), and continues, so several lexical errors can be reported together.
- UTF-8 input is validated; invalid UTF-8 is a lexical error. A byte-order mark at the start of a file is skipped.

## Parser

A hand-written recursive-descent parser that follows the grammar rule for rule, with at most two tokens of lookahead.

- **Expressions** use one function per precedence level (or precedence climbing driven by a table that matches the reference), with the special cases from the grammar: non-associative comparisons, right-associative `**` with a unary right operand, and `as` between unary and multiplicative.
- **Statement start:** an identifier followed by `=`, an assignment operator, `[` or `.` begins an assignment; followed by `(` it begins a call statement.
- **Struct literals** are recognized when an identifier is immediately followed by `{` in expression position.
- **Error recovery.** After a syntax error the parser skips to the next `;` or `}` at the same nesting level (or to the next top-level `fn`, `struct` or `let`) and continues. It always returns an AST, possibly with `Error` nodes, so semantic analysis could in principle run on partial programs. The driver does not run it, but tools such as editors will want this later.
- A missing `;` or `)` is reported at the end of the previous token, not at the next one.

## AST

One class hierarchy with a `kind` field per node, allocated in the arena (D41).

| Group | Nodes |
|-------|-------|
| Declarations | `FuncDecl`, `StructDecl`, `ConstDecl` |
| Statements | `VarDecl`, `Assign`, `CallStmt`, `If`, `Switch`, `While`, `For`, `Return`, `Break`, `Continue`, `Block` |
| Expressions | `IntLiteral`, `FloatLiteral`, `BoolLiteral`, `CharLiteral`, `StringLiteral`, `FString`, `ArrayLiteral`, `StructLiteral`, `Identifier`, `Call`, `Index`, `Field`, `Unary`, `Binary`, `Cast` |
| Type expressions | `NamedType`, `ArrayType` |

**Annotation in place.** Semantic analysis fills fields that start empty:

- every expression gets `Type* type`
- an `Identifier` gets the declaration it refers to (`Decl* resolved`)
- a `Call` gets the function it calls
- a literal that adapted to a type has its final type set (so codegen never repeats the adaptation rules)
- constant expressions get their evaluated value
- every function gets the list of its local variables (for allocating stack slots)

**Traversal.** A visitor base class for read-only walks (printing, flow analysis, lowering). The AST printer produces a stable text form used by the golden tests (`--dump-ast`).

## Semantic Analysis

Input: the AST of the whole program. Output: a `CheckedProgram` (D40, D41).

```cpp
struct CheckedProgram {
    Module&                 module;      // the annotated AST
    TypeContext&            types;       // all interned types
    FuncDecl*               main;        // validated entry point
    std::vector<StructDecl*> structs;    // in dependency order (a struct after the structs it contains)
    std::vector<FuncDecl*>  functions;
    std::vector<ConstDecl*> constants;   // in evaluation order, with values
};
```

### Types

Types are interned: two equal types are the same pointer. Kinds: primitives (`i32`, `i64`, `f32`, `f64`, `bool`, `char`, `string`, `void`), `Array{element, length or dynamic}` (recursive for arrays of arrays, D35), `Struct{decl}`, and an `Error` type used to avoid cascading errors after a failure. There is no function type value, since functions are not first-class (D30).

### Passes

1. **Collect.** Declare every top-level name; report duplicates and clashes with built-ins (D39). Resolve struct field types; report unknown types, `void` fields and recursive structs; order structs by dependency.
2. **Signatures.** Resolve parameter and return types of every function. Check `main` (D39).
3. **Constants.** Check each top-level `let const` on demand, so one constant can use another in any order. A dependency cycle is an error. Initializers are evaluated by the comptime interpreter.
4. **Bodies.** Check each function body: scopes and shadowing (D32, D37), expression types, literal adaptation, assignment targets, `const` and parameter immutability, `break`/`continue`, `switch` rules, conversions, call arguments, and struct and array literals.
5. **Flow analysis.** On the structured AST (no CFG is needed because ZackLang has no `goto`):
   - *definitely assigned* before reads (D29, D39: `if` needs `else`, `switch` needs `finally`, loops never count);
   - *all paths return* for non-`void` functions (D39).

### Typing expressions: expected types

Literal adaptation (D8, D38) needs information to flow both ways, so expressions are checked with an optional *expected type*:

```
checkExpr(expr, expected /* may be null */) → Type*
```

- A literal checked with an expected integer type becomes that type if it fits; with no expected type it becomes `i32` or `f64`.
- For a binary operator, the non-literal operand is checked first and its type becomes the expected type of a literal on the other side. When both are literals, the context's expected type is used.
- An array literal passes the expected element type to its elements and checks its length against a fixed-size expected type.
- Variable declarations, assignments, call arguments, returns, struct fields and `case` values all supply an expected type.

### Constant expressions

Array sizes, `case` values and top-level constants must be compile-time constants. Semantic analysis asks the comptime interpreter to evaluate them. The result is stored on the AST node so codegen reads the value instead of recomputing it.

## Comptime Interpreter

A tree-walking interpreter over the checked AST (D4, D27). It is used by semantic analysis for constants, array sizes, `case` values and `comptime` function calls with constant arguments.

- **Values:** integer (with its type), float, bool, char, string, array, struct. Values are separate from the compiler's runtime representation.
- **Environment:** a stack of frames mapping variables to values; each call pushes a frame. Callees never see the caller's variables (lexical scoping).
- **Limits:** a step budget and a recursion depth limit. Exceeding either reports `C` error with a call stack as notes. The defaults are configurable on the command line.
- **Purity:** the interpreter simply does not implement I/O or built-ins that have effects; `print`, `prompt` and `panic` inside a `comptime` function are rejected by semantic analysis before evaluation.
- **Errors:** division by zero, overflow, out-of-range index and invalid casts are compile-time errors (D27, rule 6). Note that overflow is always an error at compile time, even though release builds wrap at run time.
- **Shared semantics.** Arithmetic, conversions (`as`), comparison and string operations are implemented once in a small helper module in `comptime/` and are specified to match the runtime exactly. See the differential tests below.

## Code Generation

Code generation has two layers so that the language rules and the LLVM details stay apart.

| Layer | Knows about | Does not know about |
|-------|-------------|---------------------|
| **Lowering** | ZackLang semantics: checks, desugaring, value types | LLVM types and function names |
| **Backend** | the LLVM 20 C API | ZackLang |

### Backend

The only code that includes `llvm-c/*` (D3). A small class with RAII wrappers for the LLVM context, module, builder and target machine. It offers helper methods such as `emitAdd`, `emitCall`, `emitBranch` and `emitAlloca`, hides the verbose C API names, and can produce an object file, textual IR (`--emit-llvm`) or bitcode. If the C API proves unsuitable, only this class changes. LLVM IR text and the C API are pinned to LLVM 20 (D2).

### Type mapping

| ZackLang | LLVM representation |
|----------|---------------------|
| `i32`, `i64` | `i32`, `i64` |
| `f32`, `f64` | `float`, `double` |
| `bool` | `i1` in registers, `i8` in memory |
| `char` | `i32` (a Unicode scalar value) |
| `string` | `{ ptr, i64 }`: pointer to UTF-8 bytes and a byte length; immutable |
| `T[N]` | `[N x T]` |
| struct | an LLVM struct with fields in declaration order |
| `T[]` | `{ ptr, i64 length, i64 capacity }` (provisional until the memory model is decided, D5) |

`len(string)` counts Unicode scalar values, so it scans the bytes; a stored character count may be added later if profiling shows it matters.

### Lowering rules

- **Locals:** each local variable gets an `alloca` in the function's entry block. LLVM's `mem2reg` pass turns them into SSA registers. No SSA construction in our code.
- **Value types:** copying a fixed array or struct is a load/store or `memcpy` of the whole value.
- **Debug builds** (D19, D31): integer `+ - *` use the `llvm.*.with.overflow` intrinsics and call `zack_panic_overflow` on overflow; array indexing checks bounds; division and modulo check for zero.
- **Release builds:** integer `+ - *` are plain wrapping instructions. They are emitted **without** the `nsw`/`nuw` flags, since the language defines wrapping. Bounds and zero-division checks stay in (D17, D21).
- **Division edge case:** `INT_MIN / -1` and `INT_MIN % -1` are undefined in LLVM's `sdiv`/`srem`, so lowering checks for it: a panic in debug builds, the wrapped result in release builds.
- **Panics:** checks branch to a cold block that calls a `noreturn` runtime function with the source line (D31). The normal path stays straight-line so LLVM can remove checks it can prove unnecessary.
- **Conversions:** float to integer uses the saturating intrinsics (`llvm.fptosi.sat`), with `NaN` giving 0; `i32 as char` calls a validity check.
- **`**`:** a runtime helper per integer type that checks overflow in debug builds and negative exponents; floats use `llvm.pow`.
- **Equality:** strings use a runtime helper; structs and arrays use generated per-type comparison functions that compare field by field and element by element.
- **Control flow:** `if`/`elif`/`else`, `while`, `for` ranges and arrays, and `switch` (a chain of comparisons, or an LLVM `switch` instruction for integers) become basic blocks and branches. `break` and `continue` branch to the loop's exit and increment blocks.
- **F-strings:** lowered to a sequence of runtime calls that append the parts to a buffer (D43: allocation uses the never-freeing allocator for now). An f-string passed directly to `print` can be streamed without allocating.
- **Entry point:** the compiler generates the C `main` that calls the program's `main` and returns its exit code (0 for `void` main).
- **Constants** with a computed value (from the checked AST) are emitted as constants, not recomputed.
- **Verification:** after lowering, `LLVMVerifyModule` runs; a failure is an internal compiler error, never a user error.

### Optimization

`--release` runs LLVM's optimization pipeline (`default<O2>` or `default<O3>`) through `LLVMRunPasses`. `--debug` runs only `mem2reg`. There are no custom optimization passes (D6).

## Runtime Library

A static library written in C (D46), built together with the compiler and linked into every program. Its header `zackrt.h` is the contract between lowering and the runtime.

| Area | Functions (names are illustrative) |
|------|-----------------------------------|
| Output | `zack_print_i32`, `_i64`, `_f32`, `_f64`, `_bool`, `_char`, `_string` |
| Input | `zack_prompt(message)` returns a string |
| Panics | `zack_panic(message, file, line)`, `zack_panic_bounds`, `zack_panic_div_zero`, `zack_panic_overflow`, all `noreturn`; each prints to standard error and exits with code 101 (D31) |
| Strings | compare, equals, length in characters, UTF-8 validation, append for f-strings |
| Memory | `zack_alloc(size)`: a never-freeing allocator (D43), later replaced by the memory model's allocator |
| Math | integer power helpers |

Rules:

- The runtime uses only the C standard library (plus the minimum platform calls), so it builds with any C11 compiler on all three platforms.
- All output goes through UTF-8 byte writes; on Windows the runtime must write UTF-8 correctly to the console and to redirected output.
- Over time, standard-library functions move out of C into ZackLang source that ships with the compiler, leaving a small C core.

## Driver and Build Modes

`zackc` is the command-line front end (D44). Source files use the `.zl` extension.

```
zackc build file.zl -o out         compile and link (default: debug)
zackc build file.zl --release      optimized build: wrapping overflow, LLVM -O2
zackc check file.zl                stop after semantic analysis
zackc build file.zl --emit-llvm    write the LLVM IR text
zackc build file.zl --emit-obj     write the object file only
zackc --dump-tokens file.zl        print tokens (for tests)
zackc --dump-ast file.zl           print the AST (for tests)
```

| Mode | Overflow | Optimization | Checks |
|------|----------|--------------|--------|
| debug (default) | panics | `mem2reg` only | all |
| release | wraps | LLVM optimizer | bounds and zero-division remain |

- **Exit codes of `zackc`:** 0 success, 1 the program has errors, 2 bad usage or an internal compiler error.
- **Object file:** produced through the LLVM target machine for the host triple (or `--target`).
- **Linking (D51):** `zackc` invokes the system C compiler driver to link the object file, `libzackrt` and the platform C library. The command is `cc` on Unix-like systems and `clang` on Windows, and can be overridden with the `ZACK_CC` environment variable. Programs are launched with argument arrays, never through a shell, so paths with spaces work. To be revisited close to a release. See [Build and Toolchain](#build-and-toolchain) for what has and has not been verified.

## Build and Toolchain

What Phase 0 (plan items 0.1 to 0.7) has shown so far. Only macOS has been run.

### Building

```
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

- Requirements: CMake 3.20 or newer, a C++20 compiler, a C11 compiler, **LLVM 20** development files, and network access the first time you configure (CMake downloads Catch2, pinned by version and hash, D48). No Python is needed.
- CMake looks for LLVM 20 in the usual places (Homebrew `llvm@20`, `/usr/lib/llvm-20`). Pass `-DLLVM_DIR=<dir containing LLVMConfig.cmake>` to point it elsewhere. A different LLVM major version is rejected with a clear message (D2, D50).
- Options: `-DZACK_WARNINGS_AS_ERRORS=ON` (used in CI) and `-DZACK_SANITIZE=ON` (AddressSanitizer and UndefinedBehaviorSanitizer on the compiler's own code; never on `libzackrt`, D50).

### What has been verified

| Item | macOS arm64 | Linux | Windows |
|------|-------------|-------|---------|
| Builds with warnings as errors | yes (Apple clang 21, LLVM 20.1.6 from Homebrew) | yes (GitHub Actions, Ubuntu 24.04, LLVM 20 from apt.llvm.org; run 1) | yes (GitHub Actions, MSVC `cl`, LLVM 20 from conda-forge, Release; runs 2 and 3). That job does not yet treat warnings as errors |
| All 32 tests pass | yes; also under ASan and UBSan locally, and the process tests passed 20 repeats in a row | run 1: plain build 32 of 32; sanitizer job 31 of 32, the failure being a real bug (fixed). Run 2: both Linux jobs could not install LLVM (see below), so the fix is not yet confirmed | run 2: 31 of 32 (the failure was a bug in the test helper); run 3: 32 of 32 |
| `40 + 2` program exits with code 42 | yes | yes (part of the 32 tests) | yes (part of the 32 tests, runs 2 and 3) |
| `libzackrt` print and panic (exit code 101) | yes | yes (part of the 32 tests) | yes (part of the 32 tests, runs 2 and 3): the program is linked with `clang`, run, and exits with 101 |
| Latest CI result | passed (runs 1 and 2) | run 1: plain passed, sanitizers failed on the bug; runs 2 and 3: neither job got past installing LLVM because Ubuntu's package servers were unreachable (not a code failure) | green in run 3 (experimental job) |

Sources: CI runs 1 to 3 (commits `9008e6b`, `e683024`, `1fc61c2`). The Windows launcher code in `src/driver/process.cpp` compiled and worked in run 2, including capturing output, exit codes, timeouts and large inputs. The test-helper fix that followed made Windows fully green in run 3.

### LLVM C API: what the spike showed

- The whole `Backend` (declare functions, constants, `add`, calls, return, verify, run passes, print IR, emit an object file) is 229 lines of implementation behind a 77-line header that includes no LLVM header.
- Awkward parts, all absorbed by `Backend`: everything is an opaque handle and the type is passed explicitly (`LLVMBuildCall2` needs the function type); most messages are `malloc`ed strings released with `LLVMDisposeMessage`, but the message from `LLVMGetErrorMessage` is not, and has its own `LLVMDisposeErrorMessage` (see Other findings); the builder, module, context, target machine and target data each have their own dispose function; `LLVMTargetMachineEmitToFile` takes a non-const `char*` path; a call returning `void` must not be given a name.
- Error paths work: `LLVMVerifyModule` reports a block with no terminator and `LLVMRunPasses` reports an unknown pipeline, both as messages (unit tests cover both).
- The IR builder folds constants: `40 + 2` is emitted as `ret i32 42`. Tests must never depend on the shape of the generated IR (D13).
- Not yet exercised: debug information, the optimization pipeline (`default<O2>`), overflow intrinsics, structs and arrays, anything beyond `i32` and pointers.
- Gate G0: the C API worked on macOS (every run), Windows (run 3) and Linux (run 1). Nothing argued for the C++ API, so D3 stands.

### Other findings

- **LLVM version check.** LLVM's own CMake version file only accepts an exact `major.minor`, so `find_package(LLVM 20)` rejects 20.1.6 (D50).
- **Sanitizers and the runtime.** Building `libzackrt` with sanitizers made linking user programs fail with undefined `___asan_*` symbols, so the runtime is excluded (D50).
- **SIGPIPE on macOS.** A child that exits without reading a large standard input made the parent die with SIGPIPE, even with the signal blocked in the writing thread, because macOS delivers it to the whole process. `runProcessCaptured` now sets the per-descriptor `F_SETNOSIGPIPE` flag on macOS; a unit test covers it.
- **Mixing LLVM's two message-release functions is a real bug, found only on Linux.** `LLVMGetErrorMessage` returns memory allocated with `new[]`. Releasing it with `LLVMDisposeMessage` (which calls `free`) worked on macOS and in the plain Linux build, but AddressSanitizer on Linux reported an allocator mismatch. `Backend::runPasses` now uses `LLVMDisposeErrorMessage`. This is why the sanitizer job exists.
- **Packaging.** With Homebrew's LLVM, `zackc` is about 77 MB and depends at run time on Homebrew's `libz3` and `libzstd`. Plan item 10.6 will have to decide between shared LLVM, fewer components, or stripping.
- **Windows (CI runs 1 and 2).** The official LLVM Windows installer ships no CMake development files, so the experimental job takes LLVM 20 and clang from conda-forge. In the first run, installing them worked (Clang 20.1.8) and the Visual Studio environment loaded, but **Configure failed**: with conda's `clang` on `PATH`, CMake chose clang for C and MSVC for C++ and refuses to mix them. The workflow now names the compilers explicitly (`cl` for building, `clang` for linking programs through `ZACK_CC`), and in run 2 Configure and Build passed and 31 of 32 tests passed, including the end-to-end programs that are linked with `clang` against the `cl`-built runtime. Still open: the last failing test (below), whether the Windows job should also treat warnings as errors, and whether building LLVM from source (cached) is a more reliable route than conda-forge.
- **Windows text mode in test helpers.** The one failing Windows test (`runProcessCaptured feeds standard input`) printed two strings that looked identical. A C program's standard input and output are in text mode on Windows, so the test helper changed `\n` into `\r\n` itself. The helper now switches both streams to binary mode on Windows; this is a test-helper fix, and the launcher was not at fault. Confirmed by CI run 3.
- **Linux CI is exposed to package-server outages.** In run 2 both Linux jobs failed with `Temporary failure resolving azure.archive.ubuntu.com` while installing LLVM, after about 11 minutes. In run 3 both were still stuck on the same step after 12 minutes. Run 1 took 20 seconds for it. Under D52 this does not block progress. If it keeps happening, add apt retries or cache the LLVM packages.

## Cross-cutting Rules

- **Internal errors.** An unexpected state (a violated invariant, an LLVM verification failure) reports `internal compiler error` with the stage and a source location if known, and exits with code 2. It is never reported as a user error.
- **No exceptions for control flow.** Errors travel through the diagnostic engine and return values. Standard-library exceptions (such as allocation failure) are treated as fatal.
- **No global mutable state.** A `Compilation` object owns the arena, interner, source manager and diagnostics, so several compilations can run in one process (needed for tests and tools).
- **Unicode.** Identifiers are ASCII; string and char content is UTF-8 and validated at the lexer. All text handling goes through one small UTF-8 helper module.
- **Portability.** Paths, line endings (`\r\n`), the executable suffix (`.exe`) and console encoding are handled in `common/` and `driver/` only.
- **Style.** C++20, no RTTI-based dispatch on AST nodes (use the `kind` field), no raw owning pointers (arena or `unique_ptr`), warnings as errors in CI.

## Testing Strategy

Decision D13: tests check behavior, not LLVM IR text.

| Level | What is tested | How |
|-------|----------------|-----|
| Lexer | tokens for valid and invalid input | golden files of `--dump-tokens` output |
| Parser | AST shape and syntax errors | golden files of `--dump-ast` output and expected diagnostics |
| Semantic analysis | every compile-time error in the reference | test programs annotated with the expected error code and line (for example `// error: S0107`) |
| Comptime interpreter | arithmetic, limits, errors | unit tests, plus the differential tests below |
| End to end | everything the user can observe | compile with `zackc`, run, compare standard output and exit code. The programs and expected output come from [examples.md](examples.md), so the documentation is tested |
| Differential | comptime matches runtime | the same `comptime` function evaluated at compile time and at run time must give identical results; a generator produces random arithmetic and cast expressions |
| Safety | every panic | programs that must exit with code 101 and the expected message |
| Performance | speed goals (D12) | benchmark programs compared with C and Rust at each milestone |

The test suite runs on all three platforms in CI (D45).

## Extension Points

- **Mid-level IR (gate D5).** If the memory model needs a CFG-based analysis (ownership, drop elaboration), an IR is inserted between `CheckedProgram` and `Backend`. Only Lowering changes, because it is the sole consumer of `CheckedProgram` (D4).
- **Other backends.** A new backend (C, WebAssembly) implements the same lowering interface as `Backend`; the front end is unchanged.
- **Standard library in ZackLang.** Requires the driver to compile library source files and link them; the single-file `Compilation` becomes multi-file with the module system (not yet specified).
- **Editor support.** Spans, error-tolerant parsing and stable diagnostics are chosen so that a language server can reuse the front end.
- **Self-hosting (D11).** The front end uses simple data structures and no compiler-specific C++ tricks, and LLVM is reached through the C API only, so the same design can be written in ZackLang.
