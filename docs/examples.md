# ZackLang Examples

A collection of complete programs showing ZackLang features and common patterns. Each example follows the rules of the [Language Reference](language_reference.md); if you are new to the language, read the [Language Guide](language_guide.md) first.

> **Status:** the compiler does not exist yet, so these programs cannot be run today. Every example that shows an `Output` block is also meant to become an automated end-to-end test: the program is compiled, run, and its output compared with the block shown. For that reason the examples avoid anything whose output is not yet specified (such as the text form of decimal numbers).

## Contents

1. [Basics](#basics)
2. [Control Flow](#control-flow)
3. [Functions](#functions)
4. [Arrays](#arrays)
5. [Text](#text)
6. [Structs](#structs)
7. [Compile-time Evaluation](#compile-time-evaluation)
8. [Runtime Errors](#runtime-errors)
9. [Complete Programs](#complete-programs)

## Basics

### Hello World

```zacklang
fn main(): void {
    print("Hello, World!");
}
```

Output:

```text
Hello, World!
```

### Integer Arithmetic

```zacklang
fn main(): void {
    let a = 17;
    let b = 5;

    print(f"{a} + {b} = {a + b}");
    print(f"{a} - {b} = {a - b}");
    print(f"{a} * {b} = {a * b}");
    print(f"{a} / {b} = {a / b}");
    print(f"{a} % {b} = {a % b}");
    print(f"{a} ** 2 = {a ** 2}");
    print(f"-{a} / {b} = {-a / b}");
}
```

Output:

```text
17 + 5 = 22
17 - 5 = 12
17 * 5 = 85
17 / 5 = 3
17 % 5 = 2
17 ** 2 = 289
-17 / 5 = -3
```

Whole-number division truncates toward zero, which is why `-17 / 5` is `-3` and not `-4`.

### Converting Between Types

ZackLang never converts numbers implicitly. Use `as`:

```zacklang
fn main(): void {
    let total = 17;
    let count = 4;

    let whole = total / count;                       // integer division
    let average = total as f64 / count as f64;       // decimal division

    print(whole);
    print(average > 4.2);
    print((average * 100.0) as i32);

    print(3.99 as i32);                              // cuts off the decimals
    print('A' as i32);                               // the character's number
    print(66 as char);                               // the character for 66
    print(true as i32);
}
```

Output:

```text
4
true
425
3
65
B
1
```

### Literals Adapt to Their Context

```zacklang
fn main(): void {
    let big: i64 = 5_000_000_000;      // does not fit in an i32, fine as i64
    let small: f32 = 1.5;
    let n = 7;
    let m = n * 2 + 1;                 // the literals become i32 to match n

    print(big);
    print(m);
    print(0xFF);
    print(0b1010);
    print(.5 + .25 == 0.75);
}
```

Output:

```text
5000000000
15
255
10
true
```

### Reading Input

`prompt` shows a message and returns the line the user typed as a `string`:

```zacklang
fn main(): void {
    let name = prompt("What is your name? ");
    print(f"Hello, {name}!");
}
```

If the user types `Ada`:

```text
What is your name? Ada
Hello, Ada!
```

Turning input text into a number needs a standard-library function that is not yet specified, so numeric input is left out of these examples.

## Control Flow

### if / elif / else

```zacklang
fn grade(score: i32): string {
    if (score >= 90) {
        return "A";
    } elif (score >= 80) {
        return "B";
    } elif (score >= 70) {
        return "C";
    } elif (score >= 60) {
        return "D";
    } else {
        return "F";
    }
}

fn main(): void {
    let scores = [95, 83, 71, 65, 40];
    for (s in scores) {
        print(f"{s}: {grade(s)}");
    }
}
```

Output:

```text
95: A
83: B
71: C
65: D
40: F
```

### switch (no fall-through)

Only the matching branch runs. List several values with commas, and use `finally` for everything else:

```zacklang
fn day_name(day: i32): string {
    let name: string;
    switch (day) {
        case 1:
            name = "Monday";
        case 2:
            name = "Tuesday";
        case 3:
            name = "Wednesday";
        case 4:
            name = "Thursday";
        case 5:
            name = "Friday";
        case 6, 7:
            name = "Weekend";
        finally:
            name = "Invalid day";
    }
    return name;
}

fn main(): void {
    print(day_name(1));
    print(day_name(5));
    print(day_name(7));
    print(day_name(9));
}
```

Output:

```text
Monday
Friday
Weekend
Invalid day
```

Because every branch, including `finally`, assigns `name`, the compiler accepts `return name;`. Removing `finally` would make it a compile-time error: `name` might not be assigned.

### while

```zacklang
fn main(): void {
    let current = 3;
    while (current > 0) {
        print(f"{current}...");
        current -= 1;
    }
    print("Liftoff!");
}
```

Output:

```text
3...
2...
1...
Liftoff!
```

### for with a Range

The range `{start:end}` includes `start` and excludes `end`:

```zacklang
fn main(): void {
    for (i in {1:4}) {
        print(f"{i} x 7 = {i * 7}");
    }
}
```

Output:

```text
1 x 7 = 7
2 x 7 = 14
3 x 7 = 21
```

### break and continue

```zacklang
fn main(): void {
    for (i in {0:100}) {
        if (i % 2 == 0) {
            continue;          // skip even numbers
        }
        if (i > 7) {
            break;             // stop the loop
        }
        print(i);
    }
}
```

Output:

```text
1
3
5
7
```

### A Loop That Runs Until a Condition

The Collatz sequence: halve even numbers, otherwise triple and add one. Count the steps until reaching 1.

```zacklang
fn collatz_steps(start: i32): i32 {
    let n = start;
    let steps = 0;
    while (n != 1) {
        if (n % 2 == 0) {
            n /= 2;
        } else {
            n = 3 * n + 1;
        }
        steps += 1;
    }
    return steps;
}

fn main(): void {
    print(collatz_steps(6));
    print(collatz_steps(27));
}
```

Output:

```text
8
111
```

## Functions

### Recursion

```zacklang
fn gcd(a: i64, b: i64): i64 {
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}

fn main(): void {
    print(gcd(48, 18));
}
```

Output:

```text
6
```

### Iteration With a Wider Type

`i32` overflows around 2 billion, so Fibonacci numbers use `i64`:

```zacklang
fn fibonacci(n: i32): i64 {
    let a: i64 = 0;
    let b: i64 = 1;
    for (i in {0:n}) {
        let next = a + b;
        a = b;
        b = next;
    }
    return a;
}

fn main(): void {
    print(fibonacci(10));
    print(fibonacci(50));
}
```

Output:

```text
55
12586269025
```

### Predicates and Mutual Recursion

Functions may be used before they are defined, so two functions can call each other:

```zacklang
fn is_even(n: i32): bool {
    if (n == 0) {
        return true;
    }
    return is_odd(n - 1);
}

fn is_odd(n: i32): bool {
    if (n == 0) {
        return false;
    }
    return is_even(n - 1);
}

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

fn main(): void {
    print(is_even(10));
    print(is_odd(10));

    let count = 0;
    for (n in {0:30}) {
        if (is_prime(n)) {
            count += 1;
        }
    }
    print(f"{count} primes below 30");
}
```

Output:

```text
true
false
10 primes below 30
```

### Exit Codes

A `main` that returns `i32` sets the exit code:

```zacklang
fn main(): i32 {
    print("finished");
    return 3;
}
```

Output (the process exits with code 3):

```text
finished
```

## Arrays

### Fixed-size Arrays

```zacklang
fn largest(values: i32[6]): i32 {
    let best = values[0];
    for (v in values) {
        if (v > best) {
            best = v;
        }
    }
    return best;
}

fn main(): void {
    let numbers = [4, 8, 15, 16, 23, 42];       // inferred as i32[6]
    print(largest(numbers));
    print(len(numbers));
}
```

Output:

```text
42
6
```

### Arrays Are Values

Assigning or passing a fixed-size array copies it, so a function can build a modified copy without touching the original:

```zacklang
fn reversed(values: i32[5]): i32[5] {
    let result = values;                // a copy
    for (i in {0:5}) {
        result[i] = values[4 - i];
    }
    return result;
}

fn main(): void {
    let original = [1, 2, 3, 4, 5];
    let r = reversed(original);
    print(f"{original[0]} {original[1]} {original[2]} {original[3]} {original[4]}");
    print(f"{r[0]} {r[1]} {r[2]} {r[3]} {r[4]}");
}
```

Output:

```text
1 2 3 4 5
5 4 3 2 1
```

### Sorting

```zacklang
fn sorted(values: i32[6]): i32[6] {
    let a = values;
    for (i in {0:5}) {
        for (j in {0:5 - i}) {
            if (a[j] > a[j + 1]) {
                let tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
            }
        }
    }
    return a;
}

fn main(): void {
    let s = sorted([5, 2, 9, 1, 7, 3]);
    for (v in s) {
        print(v);
    }
}
```

Output:

```text
1
2
3
5
7
9
```

### Arrays of Arrays

`i32[3][3]` is 3 rows of 3 columns. Looping over an array of arrays visits each row:

```zacklang
fn main(): void {
    let table: i32[3][3] = [[0, 0, 0], [0, 0, 0], [0, 0, 0]];

    for (r in {0:3}) {
        for (c in {0:3}) {
            table[r][c] = (r + 1) * (c + 1);
        }
    }

    for (row in table) {
        print(f"{row[0]} {row[1]} {row[2]}");
    }
}
```

Output:

```text
1 2 3
2 4 6
3 6 9
```

### Dynamic Arrays

A dynamic array type `i32[]` accepts any length, including an empty array:

```zacklang
fn total(values: i32[]): i32 {
    let sum = 0;
    for (v in values) {
        sum += v;
    }
    return sum;
}

fn main(): void {
    print(total([1, 2, 3, 4]));
    print(total([]));
}
```

Output:

```text
10
0
```

Creating an array of a size that is only known from a constant, with a repeated initial value, is not yet specified. For now an array is created from a literal.

## Text

### Characters, Strings and Escapes

```zacklang
fn main(): void {
    let word = "héllo";
    print(len(word));                  // characters, not bytes
    print(word == "héllo");
    print("apple" < "banana");
    print('a' < 'b');
    print("She said \"hi\"");
    print("one\ttwo");
    print('\u{41}');
}
```

Output (the gap in the sixth line is a tab):

```text
5
true
true
true
She said "hi"
one	two
A
```

### f-strings

```zacklang
fn describe(command: string): string {
    switch (command) {
        case "start", "go":
            return "Starting";
        case "stop":
            return "Stopping";
        finally:
            return "Unknown command";
    }
}

fn main(): void {
    let name = "Ada";
    print(f"{name} has {len(name)} letters");
    print(f"2 + 3 = {2 + 3}");
    print(f"literal braces: {{ and }}");
    print(f"{describe("go")}, {describe("stop")}, {describe("jump")}");
}
```

Output:

```text
Ada has 3 letters
2 + 3 = 5
literal braces: { and }
Starting, Stopping, Unknown command
```

`switch` works on strings. The final `switch` in `describe` returns on every branch, including `finally`, so the compiler knows every path returns.

## Structs

### Defining and Using Structs

```zacklang
struct Point {
    x: i32,
    y: i32,
}

fn distance_squared(a: Point, b: Point): i32 {
    let dx = a.x - b.x;
    let dy = a.y - b.y;
    return dx * dx + dy * dy;
}

fn main(): void {
    let origin = Point { x: 0, y: 0 };
    let corner = Point { x: 3, y: 4 };
    print(distance_squared(origin, corner));
}
```

Output:

```text
25
```

### Structs Are Values

```zacklang
struct Point {
    x: i32,
    y: i32,
}

fn main(): void {
    let a = Point { x: 1, y: 2 };
    let b = a;               // a copy
    b.x = 10;
    print(a.x);
    print(b.x);
    print(a == b);
    print(a == Point { x: 1, y: 2 });
}
```

Output:

```text
1
10
false
true
```

### Nested Structs and Arrays of Structs

```zacklang
struct Point {
    x: i32,
    y: i32,
}

struct Rect {
    origin: Point,
    width: i32,
    height: i32,
}

struct Student {
    name: string,
    score: i32,
}

fn average_score(students: Student[3]): i32 {
    let total = 0;
    for (s in students) {
        total += s.score;
    }
    return total / 3;
}

fn main(): void {
    let r = Rect { origin: Point { x: 1, y: 2 }, width: 10, height: 5 };
    r.origin.x = 7;
    print(f"{r.origin.x},{r.origin.y} {r.width}x{r.height}");

    let students = [
        Student { name: "Ada", score: 90 },
        Student { name: "Grace", score: 78 },
        Student { name: "Alan", score: 85 }
    ];
    for (s in students) {
        print(f"{s.name}: {s.score}");
    }
    print(f"average: {average_score(students)}");
}
```

Output:

```text
7,2 10x5
Ada: 90
Grace: 78
Alan: 85
average: 84
```

A trailing comma is allowed in struct declarations and struct literals, but not in array literals, so the last `Student` in the array has no comma after it.

## Compile-time Evaluation

A `comptime` function can run while the program is compiled. Constants and array sizes can depend on it:

```zacklang
comptime fn factorial(n: i32): i32 {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

comptime fn fibonacci(n: i32): i32 {
    let a = 0;
    let b = 1;
    for (i in {0:n}) {
        let next = a + b;
        a = b;
        b = next;
    }
    return a;
}

let const FACT_5 = factorial(5);        // 120, computed by the compiler
let const FIB_10 = fibonacci(10);       // 55
let const SIZE = factorial(3);          // 6, usable as an array size

fn main(): void {
    let buffer: i32[SIZE] = [0, 0, 0, 0, 0, 0];
    print(FACT_5);
    print(FIB_10);
    print(len(buffer));

    let n = 6;                          // not a constant
    print(factorial(n));                // runs at run time like a normal function
}
```

Output:

```text
120
55
6
720
```

## Runtime Errors

A panic prints a message with the source line to standard error and ends the program with exit code 101.

### Guarding Against a Bad Input

```zacklang
fn safe_divide(a: i32, b: i32): i32 {
    if (b == 0) {
        panic("safe_divide: the divisor is zero");
    }
    return a / b;
}

fn main(): void {
    print(safe_divide(10, 2));
    print(safe_divide(1, 0));       // panics here
    print("never printed");
}
```

Output (then the program exits with code 101):

```text
5
```

### Out-of-range Index

```zacklang
fn element(values: i32[3], i: i32): i32 {
    return values[i];
}

fn main(): void {
    let data = [10, 20, 30];
    print(element(data, 2));
    print(element(data, 3));        // panics: index 3 is out of range
}
```

Output (then the program exits with code 101):

```text
30
```

## Complete Programs

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

Output:

```text
1
2
Fizz
4
Buzz
Fizz
7
8
Fizz
Buzz
11
Fizz
13
14
FizzBuzz
```

### Bank Account

Structs are values, so functions return an updated copy:

```zacklang
struct Account {
    owner: string,
    balance: i64,
}

fn deposit(account: Account, amount: i64): Account {
    let updated = account;
    updated.balance += amount;
    return updated;
}

fn withdraw(account: Account, amount: i64): Account {
    if (amount > account.balance) {
        panic("withdraw: insufficient funds");
    }
    let updated = account;
    updated.balance -= amount;
    return updated;
}

fn main(): void {
    let account = Account { owner: "Ada", balance: 100 };
    account = deposit(account, 50);
    account = withdraw(account, 30);
    print(f"{account.owner} has {account.balance}");
}
```

Output:

```text
Ada has 120
```
