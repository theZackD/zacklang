(* A program is a sequence of statements *)
program         ::= { statement }

(* General statement rule *)
statement       ::= var_decl
                  | print_stmt
                  | prompt_stmt
                  | if_stmt
                  | switch_stmt
                  | func_def
                  | while_stmt
                  | for_stmt
                  | break_stmt
                  | continue_stmt
                  | expression_stmt
                  | block

(* Blocks are sequences of statements enclosed in braces *)
block           ::= "{" { statement } "}"

(* Variable declarations *)
var_decl        ::= "let" [ "const" ] IDENTIFIER [ ":" type ] "=" expression ";"

(* Print statement *)
print_stmt      ::= "print" "(" expression ")" ";"

(* Prompt statement *)
prompt_stmt     ::= "prompt" "(" expression ")" ";"

(* If-elif-else statement *)
if_stmt         ::= "if" "(" expression ")" block { "elif" "(" expression ")" block } [ "else" block ]

(* Switch statement with cases and optional finally *)
switch_stmt     ::= "switch" "(" expression ")" "{" { case_stmt } [ finally_stmt ] "}"
case_stmt       ::= "case" expression ":" statement
finally_stmt    ::= "finally" ":" statement

(* Function definition with optional comptime modifier *)
func_def        ::= [ "comptime" ] "fn" IDENTIFIER "(" [ parameter_list ] ")" [ ":" type ] block
parameter_list  ::= parameter { "," parameter }
parameter       ::= IDENTIFIER [ ":" type ]

(* While loop *)
while_stmt      ::= "while" "(" expression ")" block

(* For loop over a range *)
for_stmt        ::= "for" "(" IDENTIFIER "in" "{" expression ":" expression "}" ")" block

(* Loop control *)
break_stmt      ::= "break" ";"
continue_stmt   ::= "continue" ";"

(* Expression statement (for expressions used as statements) *)
expression_stmt ::= expression ";"

(* Expression grammar *)

(* Assignment (right-associative) *)
expression      ::= assignment_expr
assignment_expr ::= logical_or_expr [ "=" assignment_expr ]

(* Logical OR *)
logical_or_expr ::= logical_and_expr { "or" logical_and_expr }

(* Logical AND *)
logical_and_expr::= equality_expr { "and" equality_expr }

(* Equality operators *)
equality_expr   ::= relational_expr { ( "==" | "!=" ) relational_expr }

(* Relational operators *)
relational_expr ::= additive_expr { ( "<" | ">" | "<=" | ">=" ) additive_expr }

(* Additive operators *)
additive_expr   ::= multiplicative_expr { ( "+" | "-" ) multiplicative_expr }

(* Multiplicative operators *)
multiplicative_expr ::= unary_expr { ( "*" | "/" | "%" ) unary_expr }

(* Unary operators *)
unary_expr      ::= [ ( "-" | "+" | "not" ) ] primary_expr

(* Primary expressions include identifiers, literals, grouped expressions, or function calls *)
primary_expr    ::= IDENTIFIER
                  | NUMBER
                  | STRING
                  | "(" expression ")"
                  | function_call

function_call   ::= IDENTIFIER "(" [ argument_list ] ")"
argument_list   ::= expression { "," expression }

(* Type annotations: these are the primitive type keywords from the lexer *)
type            ::= "i32" | "i64" | "f32" | "f64" | "bool" | "char" | "string" | "void"

(* Terminals:
     IDENTIFIER, NUMBER, STRING are tokens produced by the lexer.
     STRING may support interpolation (e.g., f"number is {x}") if prefixed with 'f'.
*)

