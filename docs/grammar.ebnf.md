(_ A program is a sequence of statements _)
program ::= { statement }

(_ General statement rule _)
statement ::= var_decl
| print_stmt
| prompt_stmt
| if_stmt
| switch_stmt
| func_def
| while_stmt
| for_stmt
| break_stmt
| continue_stmt
| return_stmt
| expression_stmt
| block

(_ Blocks are sequences of statements enclosed in braces _)
block ::= "{" { statement } "}"

(_ Variable declarations _)
var_decl ::= "let" [ "const" ] IDENTIFIER [ ":" type ] "=" expression ";"

(_ Print statement _)
print_stmt ::= "print" "(" expression ")" ";"

(_ Prompt statement _)
prompt_stmt ::= "prompt" "(" expression ")" ";"

(_ If-elif-else statement _)
if_stmt ::= "if" "(" expression ")" block { "elif" "(" expression ")" block } [ "else" block ]

(_ Switch statement with cases and optional finally _)
switch_stmt ::= "switch" "(" expression ")" "{" { case_stmt } [ finally_stmt ] "}"
case_stmt ::= "case" expression ":" statement
finally_stmt ::= "finally" ":" statement

(_ Function definition with optional comptime modifier _)
func_def ::= [ "comptime" ] "fn" IDENTIFIER "(" [ parameter_list ] ")" [ ":" type ] block
parameter_list ::= parameter { "," parameter }
parameter ::= IDENTIFIER [ ":" type ]

(_ While loop _)
while_stmt ::= "while" "(" expression ")" block

(_ For loop over a range _)
for_stmt ::= "for" "(" IDENTIFIER "in" "{" expression ":" expression "}" ")" block

(_ Loop control _)
break_stmt ::= "break" ";"
continue_stmt ::= "continue" ";"

(_ Return statement _)
return_stmt ::= "return" [ expression ] ";"

(_ Expression statement (for expressions used as statements) _)
expression_stmt ::= expression ";"

(_ Expression grammar _)

(_ Assignment (right-associative) _)
expression ::= assignment_expr
assignment_expr ::= logical_or_expr [ "=" assignment_expr ]

(_ Logical OR _)
logical_or_expr ::= logical_and_expr { "or" logical_and_expr }

(_ Logical AND _)
logical_and_expr::= equality_expr { "and" equality_expr }

(_ Equality operators _)
equality_expr ::= relational_expr { ( "==" | "!=" ) relational_expr }

(_ Relational operators _)
relational_expr ::= additive_expr { ( "<" | ">" | "<=" | ">=" ) additive_expr }

(_ Additive operators _)
additive_expr ::= multiplicative_expr { ( "+" | "-" ) multiplicative_expr }

(_ Multiplicative operators _)
multiplicative_expr ::= power_expr { ( "\*" | "/" | "%" ) power_expr }

(_ Power operator (right-associative) _)
power_expr ::= unary_expr { "\*\*" power_expr }

(_ Unary operators _)
unary_expr ::= [ ( "-" | "+" | "not" ) ] primary_expr

(_ Primary expressions include identifiers, literals, grouped expressions, or function calls _)
primary_expr ::= IDENTIFIER
| NUMBER
| string_literal
| "(" expression ")"
| function_call

function_call ::= IDENTIFIER "(" [ argument_list ] ")"
argument_list ::= expression { "," expression }

(_ Type annotations: these are the primitive type keywords from the lexer _)
type ::= base_type | array_type
base_type ::= "i32" | "i64" | "f32" | "f64" | "bool" | "string" | "void"
array_type ::= base_type "[" "]"

(_ String literals with f-string interpolation _)
string_literal ::= STRING | fstring
fstring ::= "f" '"' { fstring_char | interpolation } '"'
fstring_char ::= any character except '"' or '{'
interpolation ::= "{" expression "}"
STRING ::= '"' { string_char } '"'
string_char ::= any character except '"'

(_ Terminals:
IDENTIFIER: sequence of letters, digits, and underscores, starting with a letter or underscore
NUMBER: sequence of digits with optional decimal point and exponent
_)
