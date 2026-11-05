# ZackLang Grammar

Formal grammar of ZackLang. It must stay consistent with the [Language Reference](language_reference.md); where they disagree, fix both and record the change in [DECISIONS.md](DECISIONS.md).

## Notation

```
a ::= b          rule definition
"text"           terminal (keyword, operator or punctuation)
UPPER_CASE       lexical token, defined in the lexical grammar
( a | b )        grouping and alternatives
[ a ]            optional (zero or one)
{ a }            repetition (zero or more)
(* ... *)        comment
```

The grammar is designed to be parsed by a hand-written recursive-descent parser with at most two tokens of lookahead.

## Lexical Grammar

### Whitespace and Comments

Whitespace (space, tab, carriage return, line feed) separates tokens and is otherwise ignored. Comments are ignored. The lexer always takes the longest match.

```
LINE_COMMENT  ::= "//" { any character except line feed }
BLOCK_COMMENT ::= "/*" { any character } "*/"     (* does not nest; the first "*/" ends it *)
```

An unterminated block comment is a lexical error.

### Identifiers and Keywords

```
IDENTIFIER ::= ( letter | "_" ) { letter | digit | "_" }
letter     ::= "a".."z" | "A".."Z"
digit      ::= "0".."9"
```

An `IDENTIFIER` that equals one of the following words is a keyword token instead.

```
let       const     fn        return    struct
if        elif      else      switch    case
finally   while     for       in        break
continue  comptime  as        true      false
and       or        not       xor
i32       i64       f32       f64       bool
char      string    void
```

Reserved for future use (a lexical error today): `import`, `enum`, `null`.

`print`, `prompt`, `panic` and `len` are ordinary identifiers that name built-in functions.

### Numeric Literals

```
INTEGER ::= DEC_INT | HEX_INT | BIN_INT
DEC_INT ::= digit { [ "_" ] digit }
HEX_INT ::= "0" ( "x" | "X" ) hex_digit { [ "_" ] hex_digit }
BIN_INT ::= "0" ( "b" | "B" ) bin_digit { [ "_" ] bin_digit }
FLOAT   ::= DEC_INT "." DEC_INT [ exponent ]
          | "." DEC_INT [ exponent ]                    (* .5 means 0.5 *)
          | DEC_INT exponent
exponent ::= ( "e" | "E" ) [ "+" | "-" ] DEC_INT
hex_digit ::= digit | "a".."f" | "A".."F"
bin_digit ::= "0" | "1"
```

Literals carry no sign: `-5` is the unary operator `-` applied to `5`. A `.` immediately followed by a digit starts a float literal (`.5` is `0.5`). Since no identifier can start with a digit, this never conflicts with field access (`p.x`). A literal with a trailing dot (`5.`) is not valid. An identifier character directly after a literal (for example `12abc`) is a lexical error.

### Character and String Literals

```
CHAR   ::= "'" ( char_content | ESCAPE ) "'"
STRING ::= '"' { string_content | ESCAPE } '"'
char_content   ::= any Unicode scalar value except "'", "\" and line feed
string_content ::= any Unicode scalar value except '"', "\" and line feed
ESCAPE ::= "\" ( "n" | "t" | "r" | "\" | '"' | "'" | "0" )
         | "\u{" hex_digit { hex_digit } "}"            (* 1 to 6 hex digits *)
```

String literals cannot contain a raw line feed; use `\n`.

### F-strings

An f-string is a single lexical unit. It starts with `f"` (the letter `f` immediately followed by a double quote). Inside, literal text alternates with interpolations.

```
FSTRING        ::= 'f"' { fstring_part } '"'
fstring_part   ::= fstring_text | "{{" | "}}" | ESCAPE | interpolation
fstring_text   ::= any Unicode scalar value except '"', "\", "{", "}" and line feed
interpolation  ::= "{" expression "}"
```

- `{{` and `}}` produce literal `{` and `}`.
- The expression inside `{ }` is a normal `expression`. The closing `}` is the first `}` that is not inside nested brackets, parentheses, braces or string literals. String literals (and f-strings) may therefore appear inside an interpolation.
- Format specifiers (`{x:5}`) are not supported.

How a lexer hands the parts to the parser (for example as one token carrying the parts, or as a token sequence) is an implementation choice and is defined in the implementation plan.

### Punctuation and Operators

```
(  )  {  }  [  ]  ,  ;  :  .
+  -  *  /  %  **
==  !=  <  >  <=  >=
=  +=  -=  *=  /=  %=
```

## Syntactic Grammar

### Program

A program is a sequence of top-level declarations. Statements are not allowed at top level.

```
program     ::= { declaration }
declaration ::= func_def | struct_def | const_decl
```

### Declarations

```
const_decl ::= "let" "const" IDENTIFIER [ ":" type ] "=" expression ";"

struct_def ::= "struct" IDENTIFIER "{" [ field_decl { "," field_decl } [ "," ] ] "}"
field_decl ::= IDENTIFIER ":" type

func_def   ::= [ "comptime" ] "fn" IDENTIFIER "(" [ param_list ] ")" ":" type block
param_list ::= param { "," param }
param      ::= IDENTIFIER ":" type
```

### Types

```
type      ::= elem_type { "[" [ expression ] "]" }
elem_type ::= "i32" | "i64" | "f32" | "f64" | "bool" | "char" | "string" | "void"
            | IDENTIFIER                        (* a struct name *)
```

- `T[N]` is a fixed-size array; the expression `N` must be a compile-time constant. `T[]` is a dynamic array.
- Suffixes read outermost first: `T[A][B]` is an array of `A` elements, each of type `T[B]`. So `a[i][j]` has `i < A` and `j < B`. `i32[][]` is a dynamic array of dynamic arrays and `i32[3][]` is a fixed array of 3 dynamic arrays.
- `void` is only valid as a function return type and cannot be an array element type; this is checked semantically.

### Statements

```
block     ::= "{" { statement } "}"

statement ::= var_decl
            | assign_stmt
            | call_stmt
            | if_stmt
            | switch_stmt
            | while_stmt
            | for_stmt
            | return_stmt
            | break_stmt
            | continue_stmt
            | block

var_decl    ::= "let" [ "const" ] IDENTIFIER [ ":" type ] [ "=" expression ] ";"
assign_stmt ::= lvalue assign_op expression ";"
call_stmt   ::= call ";"
return_stmt   ::= "return" [ expression ] ";"
break_stmt    ::= "break" ";"
continue_stmt ::= "continue" ";"

lvalue    ::= IDENTIFIER { "[" expression "]" | "." IDENTIFIER }
assign_op ::= "=" | "+=" | "-=" | "*=" | "/=" | "%="
```

Semantic rules enforced after parsing, not by the grammar: a `var_decl` needs a type annotation or an initializer; `let const` needs an initializer; an assignment's root variable must not be `const`.

A statement starting with an `IDENTIFIER` is an `assign_stmt` when the identifier is followed by `=`, an assignment operator, `[` or `.`, and a `call_stmt` when it is followed by `(`.

```
if_stmt ::= "if" "(" expression ")" block
            { "elif" "(" expression ")" block }
            [ "else" block ]

while_stmt ::= "while" "(" expression ")" block

for_stmt ::= "for" "(" IDENTIFIER "in" for_source ")" block
for_source ::= "{" expression ":" expression "}"        (* range, end exclusive *)
             | expression                                (* array *)

switch_stmt    ::= "switch" "(" expression ")" "{" { case_clause } [ finally_clause ] "}"
case_clause    ::= "case" expression { "," expression } ":" { statement }
finally_clause ::= "finally" ":" { statement }
```

The statements of a `case` or `finally` clause end where the next `case`, `finally` or `}` begins. These words cannot start a statement, so no ambiguity arises.

### Expressions

Precedence from lowest to highest. This mirrors the table in the language reference.

```
expression ::= or_expr

or_expr    ::= xor_expr { "or" xor_expr }
xor_expr   ::= and_expr { "xor" and_expr }
and_expr   ::= equality_expr { "and" equality_expr }

equality_expr   ::= relational_expr [ ( "==" | "!=" ) relational_expr ]
relational_expr ::= additive_expr [ ( "<" | "<=" | ">" | ">=" ) additive_expr ]

additive_expr       ::= multiplicative_expr { ( "+" | "-" ) multiplicative_expr }
multiplicative_expr ::= cast_expr { ( "*" | "/" | "%" ) cast_expr }

cast_expr  ::= unary_expr { "as" type }
unary_expr ::= ( "-" | "not" ) unary_expr
             | power_expr

power_expr ::= postfix_expr [ "**" power_rhs ]          (* right-associative *)
power_rhs  ::= "-" power_rhs
             | power_expr
```

- `equality_expr` and `relational_expr` accept at most one operator, so `a < b < c` and `a == b == c` are syntax errors.
- Because `power_expr` sits below `unary_expr`, `-2 ** 2` parses as `-(2 ** 2)`. The right side of `**` may carry a leading `-`, so `2 ** -1` is valid.
- `as` binds tighter than `*` and looser than unary operators: `-x as i64` is `(-x) as i64`, and `a * b as f64` is `a * (b as f64)`.

```
postfix_expr ::= primary { "[" expression "]" | "." IDENTIFIER }

primary ::= INTEGER
          | FLOAT
          | CHAR
          | STRING
          | FSTRING
          | "true"
          | "false"
          | array_literal
          | struct_literal
          | call
          | IDENTIFIER
          | "(" expression ")"

array_literal  ::= "[" [ expression { "," expression } ] "]"
struct_literal ::= IDENTIFIER "{" [ field_init { "," field_init } [ "," ] ] "}"
field_init     ::= IDENTIFIER ":" expression

call     ::= IDENTIFIER "(" [ arg_list ] ")"
arg_list ::= expression { "," expression }
```

- Only a plain identifier can be called; functions are not first-class values yet.
- A `struct_literal` is recognized when an `IDENTIFIER` is directly followed by `{`. In the places where an expression is followed by a block (`if`, `while`, `for`, `switch`) the expression is inside parentheses, so a `{` never directly follows it and no ambiguity arises.

## Grammar-level Rules

Rules the grammar fixes beyond what the reference spells out. They are settled (decisions D34 to D36 in [DECISIONS.md](DECISIONS.md)). Change any of them here, in the reference and in the decision log together.

1. String and character literals cannot span lines.
2. An interpolation may contain string literals and nested f-strings.
3. A float literal may start with `.` (`.5` is `0.5`) but may not end with it (`5.` is invalid).
4. Struct declarations and struct literals allow a trailing comma; parameter, argument and array-literal lists do not.
5. A bare block `{ ... }` is allowed as a statement.
6. Arrays of arrays are allowed; `T[A][B]` is an array of `A` elements of type `T[B]`.
7. `case` and `finally` bodies are statement lists without braces; each body is its own scope.
8. Unary `+` does not exist.
