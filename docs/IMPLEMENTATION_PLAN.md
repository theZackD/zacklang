# ZackLang Implementation Plan

The phased plan for building the ZackLang compiler (`zackc`). It is the checklist we work through and tick off, so we always know where we are. Design lives in [compiler_architecture.md](compiler_architecture.md), language rules in [language_reference.md](language_reference.md), and the reasons behind choices in [DECISIONS.md](DECISIONS.md).

## How to Use This Plan

**Status marks:** `[ ]` not started, `[~]` in progress, `[x]` done, `[!]` blocked (say why).

**Rules**

1. Work on one sub-phase at a time, in order, unless a note says otherwise.
2. A sub-phase is **done** only when all of these hold:
   - its **Done when** condition is met;
   - its **Tests** exist and pass in CI on every supported platform (D45);
   - the docs it touches are updated (the reference, grammar, guide or examples), doc-first when the language changes;
   - any design change is recorded in `DECISIONS.md`;
   - the code contains no debug printing to standard output, no `exit()` inside library code, no global mutable state and no unexplained `TODO`.
3. Tests check behavior, never LLVM IR text (D13).
4. When a sub-phase shows the design is wrong, stop, fix the docs and the decision log, then continue.
5. Size tags are relative effort, not dates: **S** (about a day), **M** (a few days), **L** (a week or more).

**Lessons from the legacy code that this plan guards against:** debug prints in library code, early `exit()` on errors, no real driver, stage boundaries that leaked, tests written to fit the implementation, and passes that were only ever tested against hand-made inputs.

## Progress

| Phase | Name | Milestone | Status |
|-------|------|-----------|--------|
| 0 | Foundations and spikes | G0: toolchain proven on all platforms | [ ] |
| 1 | Common infrastructure | | [ ] |
| 2 | Lexer | | [ ] |
| 3 | AST and parser | | [ ] |
| 4 | Numeric core, end to end | **M1**: numeric programs run | [ ] |
| 5 | Compile-time evaluation | | [ ] |
| 6 | Text | | [ ] |
| 7 | Arrays and structs | **M2**: every example except dynamic arrays runs | [ ] |
| 8 | Conformance and diagnostics | | [ ] |
| 9 | Memory model and dynamic features | **G2** gate, then **M3**: the whole documented language | [ ] |
| 10 | Performance and release quality | **v0.1 release** | [ ] |
| 11 | Path to self-hosting | **M4**: the compiler compiles itself | [ ] |

**Current phase: 0, not started.**

**Why this order.** The front end (phases 1 to 3) is finished first because it is well specified and has no external dependency. Phase 4 then builds a thin, complete compiler for the numeric subset, so there is a real working compiler early (the priority from the start of the project). Later phases widen the language one feature group at a time, always end to end (parse, check, generate, run, test) rather than building a whole stage at a time.

## Phase 0: Foundations and Spikes

Goal: prove the toolchain works on macOS, Linux and Windows before writing real compiler code.

- [ ] **0.1 Repository skeleton** (S). CMake project, C++20 for the compiler, C11 for the runtime, the directory layout from the architecture doc, strict warnings, `.gitignore`, `.clang-format`.
  - Done when: an empty `zackc` builds with one command on each platform.
  - Tests: build only.
- [ ] **0.2 Test infrastructure** (M). A unit test framework; a golden-file runner (compare tool output with an expected file); an end-to-end runner (compile a `.zl`, run it, compare standard output and exit code with expected files); everything wired into `ctest`.
  - Done when: one sample of each test kind passes under `ctest`.
  - Tests: the samples themselves.
- [ ] **0.3 LLVM 20 integration** (M). CMake finds LLVM 20 headers and libraries on each OS and links the C API.
  - Done when: a test program prints the LLVM version string on each platform.
  - Tests: that program.
- [ ] **0.4 Backend spike** (M). Using only the C API, build `main` returning `40 + 2`, emit an object file and textual IR, link with the system compiler.
  - Done when: the produced executable exits with code 42, and notes on how awkward the C API felt are written down.
  - Tests: end-to-end test of the exit code.
- [ ] **0.5 Runtime spike** (S). A `libzackrt` with `zack_print_i32` and `zack_panic`; the spike module calls them.
  - Done when: a program prints `42`; another exits with code 101 and a message on standard error.
  - Tests: end-to-end tests of both.
- [ ] **0.6 Continuous integration** (M). CI on macOS, Linux and Windows: build, run all tests.
  - Done when: green on all three. The working link command and the source of LLVM 20 on Windows are written in the architecture doc.
  - Tests: CI itself.
- [ ] **0.7 Gate G0** (S). Decide with evidence: keep the LLVM C API (D3) or switch `Backend` to the C++ API; confirm the link strategy (D45); confirm Windows is practical.
  - Done when: D3 and D45 are confirmed or amended in the decision log and the architecture doc.

## Phase 1: Common Infrastructure

Goal: the shared pieces every stage uses. Nothing here knows about the language.

- [ ] **1.1 UTF-8 helpers** (S). Decode, encode, validate, scalar-value checks.
  - Done when: invalid sequences, surrogates, overlong forms and values above `U+10FFFF` are rejected.
  - Tests: unit tests with edge cases.
- [ ] **1.2 SourceManager and Span** (S). File storage, offset to line and column, CRLF handling, columns counted in Unicode scalar values.
  - Done when: positions are right for files with `\n`, `\r\n`, multi-byte characters and no trailing newline.
  - Tests: unit tests.
- [ ] **1.3 Diagnostics** (M). `Diagnostic`, `DiagnosticEngine` (counting, error limit, notes), the caret renderer, the error-code registry.
  - Done when: rendering matches the format in the architecture doc, including multi-byte lines; the limit stops at 20 and reports that it stopped.
  - Tests: golden tests of rendered output.
- [ ] **1.4 Arena and interner** (S). Bump allocator for AST nodes and types; identifier interning.
  - Done when: nothing leaks (checked with the sanitizers) and interning is deterministic.
  - Tests: unit tests under ASan/UBSan.
- [ ] **1.5 Compilation object** (S). One object owning source manager, arena, interner and diagnostics, with no global state.
  - Done when: two compilations can run in one process without interfering.
  - Tests: unit test running two compilations.

## Phase 2: Lexer

Goal: turn source text into tokens exactly as the lexical grammar describes.

- [ ] **2.1 Basic tokens** (M). Identifiers, keywords, reserved words, punctuation, operators with longest match, whitespace, line and block comments.
  - Done when: every token kind in the grammar is produced; `//` is never an operator; `import`, `enum`, `null` are errors; an unterminated block comment is an error.
  - Tests: golden token dumps, one file per rule, plus invalid inputs.
- [ ] **2.2 Number literals** (M). Decimal, hex, binary, `_` separators, floats with `.5`, exponents.
  - Done when: `5.`, `12abc`, `0x`, `1__0` and values above 64 bits are errors; `.5`, `1_000`, `1.5e3`, `2e-4` lex correctly.
  - Tests: golden tests, table-driven valid and invalid cases.
- [ ] **2.3 Character and string literals** (M). All escapes, `\u{...}`, UTF-8 validation, decoded values stored in the token.
  - Done when: unterminated literals, raw newlines, bad escapes, invalid scalar values and empty char literals are errors with correct spans.
  - Tests: golden tests.
- [ ] **2.4 F-strings** (M). Parts and interpolations, `{{` and `}}`, matching braces with nested strings and f-strings, absolute spans inside interpolations.
  - Done when: every rule in the f-string grammar works; an unterminated interpolation is reported at the opening brace.
  - Tests: golden tests including nesting and error cases.
- [ ] **2.5 Recovery and `--dump-tokens`** (S). Continue after a lexical error; stable token dump for tests.
  - Done when: several lexical errors in one file are all reported.
  - Tests: golden tests; a simple fuzz test showing no crash on random bytes.

## Phase 3: AST and Parser

Goal: parse the full language from the grammar, with good syntax errors.

- [ ] **3.1 AST and printer** (M). All node kinds, the visitor base, a stable text form for `--dump-ast`.
  - Done when: every grammar construct has a node and prints deterministically.
  - Tests: unit tests of the printer.
- [ ] **3.2 Expressions** (L). Every precedence level, non-associative comparisons, right-associative `**` with a unary right operand, `as`, postfix call, index and field, all literals, array and struct literals.
  - Done when: the precedence table in the reference is verified case by case, including `-2 ** 2`, `a < b < c` (error), `-x as i64`, `a * b as f64`.
  - Tests: golden AST dumps, one per precedence rule.
- [ ] **3.3 Types and declarations** (M). `fn`, `comptime fn`, `struct`, top-level `let const`, type syntax including `T[N]`, `T[]` and arrays of arrays.
  - Done when: `i32[2][3]` parses as an array of 2 elements of `i32[3]`; trailing-comma rules (D36) hold.
  - Tests: golden AST dumps and error tests.
- [ ] **3.4 Statements** (M). Variable declarations, assignment and compound assignment, call statements, `if`/`elif`/`else`, `while`, both `for` forms, `switch` with multi-value `case` and `finally`, `return`, `break`, `continue`, blocks.
  - Done when: the statement-start disambiguation (assignment versus call) works; `case`/`finally` bodies end correctly.
  - Tests: golden AST dumps.
- [ ] **3.5 F-string interpolation parsing** (S). Parse each interpolation with a sub-parser.
  - Done when: spans of errors inside interpolations point at the exact file position.
  - Tests: golden tests.
- [ ] **3.6 Recovery and syntax diagnostics** (M). Recover at `;`, `}` and top-level keywords; error messages for common mistakes (missing `;`, missing `)`, `=` instead of `==`, assignment as an expression).
  - Done when: a file with several unrelated syntax errors reports each once, without cascades.
  - Tests: golden diagnostic tests; every complete program in `examples.md` parses; a fuzz test (random token sequences) never crashes or hangs.

## Phase 4: Numeric Core, End to End (milestone M1)

Goal: a real compiler for programs that use numbers, booleans, variables, control flow and functions. This is the first working compiler. Everything here is checked, lowered, linked and run.

- [ ] **4.1 Walking skeleton** (L). Minimal semantic analysis (types, scopes), the `Backend` class, the lowering for `fn main(): void { print(<integer expression>); }`, the `zackc build` driver, the runtime `print`.
  - Done when: `zackc build hello.zl -o hello && ./hello` prints the expected number on all platforms.
  - Tests: end-to-end tests; the driver's exit codes (0, 1, 2).
- [ ] **4.2 Types, inference and literal adaptation** (L). `i32`, `i64`, `f32`, `f64`, `bool`; expected-type checking; inference; literal range errors; the `as` cast table; `print` of `i32`, `i64` and `bool` (decimal output is not printed in tests until its format is specified).
  - Done when: every case in "Literal Adaptation" and "Conversions" of the reference behaves as written, including `-2147483648` and `let f: f64 = 1;` (error).
  - Tests: one semantic test per rule (expected error codes) and end-to-end tests for each conversion result.
- [ ] **4.3 Variables, constants, scope** (M). `let`, `let const`, assignment and compound assignment, shadowing, duplicate and unknown names, `const` and parameter immutability.
  - Done when: all scope rules (D32, D37) and mutability rules have tests.
  - Tests: semantic tests and end-to-end tests.
- [ ] **4.4 Operators** (M). Arithmetic, comparison and logical operators, short-circuit, unary operators, `**`, floating-point semantics.
  - Done when: the evaluation-order and short-circuit rules are verified with side-effecting calls; `-7 / 2` and `-7 % 2` give `-3` and `-1`.
  - Tests: end-to-end tests of each operator and precedence level.
- [ ] **4.5 Control flow** (M). `if`/`elif`/`else`, `while`, `for` over ranges, `break`, `continue`.
  - Done when: a range with `start >= end` runs zero times; bounds are evaluated once; `break` outside a loop is an error.
  - Tests: end-to-end and semantic tests.
- [ ] **4.6 Functions** (M). Declarations, calls, forward references, recursion, mutual recursion, argument adaptation, `void`, `return`, `main` validation and exit code.
  - Done when: every rule in "Functions" and "Program Structure" is tested.
  - Tests: end-to-end tests; semantic tests for `main` errors.
- [ ] **4.7 Flow analysis** (M). Definite assignment and all-paths-return (D39).
  - Done when: each rule is tested positively and negatively, including `if` without `else`, loops that never count and variables assigned on every branch.
  - Tests: semantic tests.
- [ ] **4.8 Runtime checks and build modes** (L). Division and modulo by zero, `INT_MIN / -1`, overflow checks in debug builds, wrapping in release builds, `integer ** negative`, panics with exit code 101 and a message with the source line.
  - Done when: the same program panics in debug and wraps in release exactly as D19 says; every panic cause for numbers is tested.
  - Tests: end-to-end tests in both modes, expecting exit code 101 or wrapped results.
- [ ] **4.9 Milestone M1 review** (S). Run the numeric programs from `examples.md` that need no text: Literals Adapt, break and continue, Collatz, gcd, fibonacci.
  - Done when: they pass in debug and release on all platforms.

## Phase 5: Compile-time Evaluation

Goal: the comptime interpreter and everything that depends on constants.

- [ ] **5.1 Interpreter core** (L). Values, environments, expression evaluation for numbers and booleans; the shared arithmetic and conversion helpers.
  - Done when: arithmetic, comparisons, casts and errors (overflow, division by zero) match the run-time rules (overflow is always an error at compile time).
  - Tests: unit tests.
- [ ] **5.2 Statements, calls, limits** (M). `if`, `while`, `for`, locals, `return`, calls and recursion; step budget and recursion depth with a call-stack note.
  - Done when: an infinite loop and runaway recursion each end in a compile-time error, not a hang.
  - Tests: unit tests and semantic tests.
- [ ] **5.3 Integration with semantic analysis** (M). Top-level `let const` with any-order dependencies and cycle detection; `comptime fn` purity checks; calls with constant arguments evaluated by the compiler; non-constant arguments run at run time.
  - Done when: every rule in "Compile-time Evaluation" has a test.
  - Tests: semantic and end-to-end tests.
- [ ] **5.4 Differential tests** (M). A generator of random arithmetic, cast and control-flow functions; each is evaluated at compile time and at run time and the results compared.
  - Done when: thousands of random cases agree, with failures reproducible by seed.
  - Tests: the differential test suite itself, run in CI with a fixed seed set.

## Phase 6: Text

Goal: characters, strings, `switch`, f-strings and input.

- [ ] **6.1 `char` and `string`** (M). Literals, `print` of `char` and `string`, `len`, correct UTF-8 output on all platforms.
  - Done when: non-ASCII text prints correctly to a terminal and to a redirected file on Windows, macOS and Linux.
  - Tests: end-to-end tests with non-ASCII text.
- [ ] **6.2 Comparison and conversion** (S). `==`, `!=`, ordering of strings and chars, `as` between `char` and integers with the validity check.
  - Done when: lexicographic ordering by scalar value is verified; an invalid `i32 as char` panics.
  - Tests: end-to-end tests.
- [ ] **6.3 `switch`** (M). Integer, `char`, `bool` and `string` subjects; multi-value `case`; duplicate-value errors; `finally`; no fall-through; interaction with definite assignment and all-paths-return; comptime support for `case` values.
  - Done when: every rule in the `switch` section has a test, including `break` inside `switch` referring to the loop.
  - Tests: semantic and end-to-end tests.
- [ ] **6.4 F-strings** (M). Lowering through runtime append helpers using the never-freeing allocator (D43); `{{ }}`; nesting; streaming into `print` without allocation.
  - Done when: all f-string examples in the reference and guide produce the documented text.
  - Tests: end-to-end tests including nested f-strings.
- [ ] **6.5 `prompt` and `panic(message)`** (S). Input reading, message printing, the message format on standard error.
  - Done when: `prompt` works with piped input and at end of input; `panic` exits with code 101.
  - Tests: end-to-end tests with input files.
- [ ] **6.6 Examples enabled** (S). Make the text-based sections of `examples.md` pass (see the table at the end).

## Phase 7: Arrays and Structs (milestone M2)

Goal: aggregate types with value semantics.

- [ ] **7.1 Fixed-size arrays** (L). Types, literals with adaptation, indexing, bounds checks, assignment and copying, `len`, size from constants.
  - Done when: length mismatches and empty literals without an expected type are compile-time errors; runtime out-of-range indexing panics in both modes (a constant index that is provably out of range for a fixed-size array may additionally be reported at compile time).
  - Tests: semantic and end-to-end tests.
- [ ] **7.2 Nested arrays and `for` over arrays** (M). `T[A][B]` layout (row-major), `for (x in array)`, equality.
  - Done when: `i32[2][3]` indexes as `a[row][col]` and copies as a single value.
  - Tests: end-to-end tests.
- [ ] **7.3 Structs** (L). Declaration checks (duplicate fields, recursion, ordering), literals, field access and assignment, value semantics, equality (generated per-type functions).
  - Done when: copies are independent, `==` compares field by field, recursive structs are rejected.
  - Tests: semantic and end-to-end tests.
- [ ] **7.4 Combinations** (M). Arrays of structs, structs with arrays, nested structs, deep `const` immutability, parameters of aggregate type.
  - Done when: assignments through nested paths work and constant aggregates cannot be modified.
  - Tests: end-to-end and semantic tests.
- [ ] **7.5 Aggregates in comptime** (M). Interpreter support for fixed arrays and structs; `comptime` examples with array sizes.
  - Done when: differential tests cover arrays and structs.
- [ ] **7.6 Milestone M2 review** (S). Every program in `examples.md` that does not need dynamic arrays passes in debug and release on all platforms.

## Phase 8: Conformance and Diagnostics

Goal: make sure the compiler does what the documents say, no more and no less.

- [ ] **8.1 Conformance matrix** (L). A table mapping every rule and every item of "Compile-time Errors at a Glance" and "Runtime Errors" in the reference to at least one test; fill the gaps.
  - Done when: no row of the matrix is untested.
  - Tests: the matrix itself is checked in CI (every referenced test exists and passes).
- [ ] **8.2 Diagnostics quality** (M). Clear messages and help text for the common beginner mistakes listed in the guide; notes with secondary spans; Unicode-correct carets.
  - Done when: each entry in the guide's "Common Mistakes" table produces a helpful, tested message.
  - Tests: golden diagnostic tests.
- [ ] **8.3 Documentation sync** (M). Fix every difference between docs and behavior found so far; every `zacklang` program in the docs that is a complete program is compiled in CI.
  - Done when: the doc tests are green.
- [ ] **8.4 Robustness** (M). Fuzzing of the lexer, parser and semantic analysis; internal-error path; sanitizer runs.
  - Done when: a long fuzz run finds no crash, hang or sanitizer report.

## Phase 9: Memory Model and Dynamic Features (gate G2)

Goal: decide the memory model, then build what depends on it. This phase starts with a decision, not with code.

- [ ] **9.1 Gate G2: decisions** (M). Choose the memory model (GC, reference counting, ownership and borrowing, manual with checks); decide whether a mid-level IR is needed (D5); decide dynamic array copy or share semantics, string concatenation, the array-fill syntax, and the scope of the standard library. Compare options with small prototypes and benchmarks.
  - Done when: each decision is in `DECISIONS.md` with its reasons, and the reference and guide are updated.
- [ ] **9.2 Mid-level IR** (L, only if 9.1 requires it). Insert it between `CheckedProgram` and `Backend`.
  - Done when: all existing tests pass unchanged (they test behavior, D13).
- [ ] **9.3 Real allocator** (M). Replace the never-freeing allocator (D43) as the model requires.
  - Done when: leak and use-after-free checks pass on the whole test suite.
- [ ] **9.4 Dynamic arrays** (L). Type, literals, indexing, `len`, growth operations, `for` over them, copy or share semantics.
  - Done when: the dynamic array sections of the guide and examples pass.
- [ ] **9.5 String and number operations** (L). Concatenation, parsing and formatting of numbers, other helpers chosen in 9.1; array creation with a repeated value.
  - Done when: every helper has tests; the "not yet specified" notes in the docs are resolved.
- [ ] **9.6 Milestone M3 review** (S). The whole documented language is implemented. All examples pass.

## Phase 10: Performance and Release Quality

Goal: meet the speed goal (D12) and ship version 0.1.

- [ ] **10.1 Release pipeline** (M). LLVM optimization levels, release semantics verified.
  - Done when: release builds pass the whole test suite with wrapping semantics.
- [ ] **10.2 Benchmarks** (L). A benchmark suite (loops, recursion, floating-point, array and struct work, sorting, string building) with C and Rust equivalents; results recorded. Numeric targets are set after the first measurements, not guessed now.
  - Done when: the suite runs in CI on demand and results are stored.
- [ ] **10.3 Optimization work** (L). Fix what the benchmarks reveal: check elimination, copy avoidance, attribute emission.
  - Done when: the agreed targets are met or the remaining gaps are written down with reasons.
- [ ] **10.4 Compile-time performance** (M). Measure the compiler itself on large generated inputs.
  - Done when: a reasonable build-time budget is defined and met.
- [ ] **10.5 Debug information** (M). Source locations so debuggers show lines and variables.
  - Done when: a debugger can set a breakpoint on a source line and show local variables on macOS and Linux (Windows as supported by the platform).
- [ ] **10.6 Packaging and release** (M). Install layout, release artifacts for the three platforms, README, version command, release notes.
  - Done when: a clean machine can install a release and compile and run the guide's first program.
- [ ] **10.7 v0.1 checklist** (S). All docs consistent, all tests green, no open `[!]` items.

## Phase 11: Path to Self-hosting

Goal: write the ZackLang compiler in ZackLang. Starts after v0.1 (D11). Its sub-phases are refined when we get there.

- [ ] **11.1 Requirements study** (M). List every language and library feature the compiler needs (pointers or references, modules, enums and tagged unions, generics, error handling, file I/O, maps, calling the LLVM C API through an FFI).
- [ ] **11.2 Specify and build each feature** (L, repeated). One feature at a time: decision, docs, implementation, tests.
- [ ] **11.3 Standard library in ZackLang** (L). Move runtime and library code out of C where possible.
- [ ] **11.4 Port the front end** (L). Lexer first, then parser, then semantic analysis; each port must produce byte-identical output to the C++ version on the whole test suite.
- [ ] **11.5 Bootstrap** (M). Stage 1: the C++ compiler builds the ZackLang compiler. Stage 2: that compiler builds itself. Stage 3: the result builds itself again and must be identical to stage 2.

## Where Each Example Becomes Passable

Sections of [examples.md](examples.md) and the phase after which they must pass.

| Examples section | Passes after |
|------------------|--------------|
| Literals Adapt to Their Context | 4 |
| break and continue; Collatz; Recursion (gcd); Iteration With a Wider Type | 4 |
| Hello World; Integer Arithmetic; Converting Between Types; Reading Input | 6 |
| switch; while; for with a Range; Predicates and Mutual Recursion; Exit Codes | 6 |
| Characters, Strings and Escapes; f-strings | 6 |
| Guarding Against a Bad Input (`safe_divide`); FizzBuzz | 6 |
| if / elif / else (uses an array); Fixed-size Arrays; Arrays Are Values; Sorting | 7 |
| Arrays of Arrays | 7 |
| Structs (all three); Compile-time Evaluation; Out-of-range Index; Bank Account | 7 |
| Dynamic Arrays | 9 |

## Gates and Milestones

| Name | After | What it proves |
|------|-------|----------------|
| **G0** | Phase 0 | The toolchain (LLVM C API, runtime, linking, CI) works on all three platforms |
| **M1** | Phase 4 | A real compiler: numeric programs compile and run in debug and release |
| **M2** | Phase 7 | The documented language minus dynamic arrays, string operations and the final memory model |
| **G2** | start of Phase 9 | Memory model and IR decisions made with evidence |
| **M3** | Phase 9 | The whole documented language works |
| **v0.1** | Phase 10 | Fast, packaged and documented |
| **M4** | Phase 11 | The compiler compiles itself |

## Risks

| Risk | Effect | Mitigation |
|------|--------|------------|
| LLVM C API awkward or incomplete (D3) | Backend slower to write | Phase 0 spike and gate G0; only `Backend` would change |
| Windows toolchain (LLVM 20 libraries, linker) | Blocks one platform | Verified in phase 0; CI from the start |
| Memory model chosen late (D5) | Rework in strings and arrays | Never-freeing allocator in the meantime (D43); behavior tests survive a change; gate G2 before dynamic features |
| Two implementations of the same semantics (comptime and runtime) | Silent differences | Shared helper module and differential tests (5.4) |
| Scope growth | The plan never finishes | Features not in the docs go to the decision log first; self-hosting features wait for phase 11 |
| Docs and compiler drift apart | Wrong expectations | Doc programs compiled in CI (8.3); conformance matrix (8.1) |
