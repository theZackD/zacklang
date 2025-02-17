#ifndef AST_H
#define AST_H

// Enumeration for AST node types
typedef enum
{
  AST_VAR_DECL,    // Variable declaration
  AST_PRINT_STMT,  // Print statement
  AST_PROMPT_STMT, // Prompt statement
  AST_IF_STMT,     // If statement (includes else)
  AST_WHILE_STMT,  // While loop
  AST_FOR_STMT,    // For loop (range-based)
  AST_FUNC_DEF,    // Function definition
  AST_EXPR_STMT,   // Expression used as a statement
  AST_BLOCK,       // Block of statements
  AST_BINARY_EXPR, // Binary expression
  AST_UNARY_EXPR,  // Unary expression
  AST_LITERAL,     // Literal (number, string, etc.)
  AST_IDENTIFIER,  // Identifier
  AST_FUNC_CALL,   // Function call
  AST_ASSIGN_EXPR, // Assignment expression
  AST_RETURN_STMT  // Return statement
} ASTNodeType;

// Forward declaration
typedef struct ASTNode ASTNode;

// Definition for an AST node
struct ASTNode
{
  ASTNodeType type;
  union
  {
    // Variable declaration: let [const] id [: type] = initializer;
    struct
    {
      int is_const; // 1 for immutable ("const"), 0 for mutable
      char *identifier;
      char *type_annotation; // Initially stored as a string; later, could be a StaticType*
      ASTNode *initializer;  // Expression node
    } var_decl;

    // Print statement: print(expr);
    struct
    {
      ASTNode *expr;
    } print_stmt;

    // Prompt statement: prompt(expr);
    struct
    {
      ASTNode *expr;
    } prompt_stmt;

    // If statement: if (cond) { ... } [elif (cond) { ... }] [else { ... }]
    // For simplicity, this node includes one condition, an "if" block, and an optional "else" block.
    // You can later expand it to support multiple elif branches.
    struct
    {
      ASTNode *condition;
      ASTNode *if_block;
      ASTNode *else_block; // NULL if not present
    } if_stmt;

    // While loop: while (cond) { ... }
    struct
    {
      ASTNode *condition;
      ASTNode *block;
    } while_stmt;

    // For loop: for (id in {start : end}) { ... }
    struct
    {
      char *iterator;
      ASTNode *start_expr;
      ASTNode *end_expr;
      ASTNode *block;
    } for_stmt;

    // Function definition: fn name(parameters) [: return_type] { body }
    struct
    {
      char *name;
      ASTNode **parameters; // Array of parameter nodes (or identifiers)
      int param_count;
      char *return_type; // As a string initially
      ASTNode *body;     // Block node
    } func_def;

    // Expression statement: expression;
    struct
    {
      ASTNode *expr;
    } expr_stmt;

    // Block: { statement1, statement2, ... }
    struct
    {
      ASTNode **statements; // Array of statement nodes
      int stmt_count;
    } block;

    // Binary expression: left operator right
    struct
    {
      char *op;
      ASTNode *left;
      ASTNode *right;
    } binary_expr;

    // Unary expression: operator operand
    struct
    {
      char *op;
      ASTNode *operand;
    } unary_expr;

    // Literal: numbers, strings, etc.
    struct
    {
      char *value;
    } literal;

    // Identifier: variable names, function names, etc.
    struct
    {
      char *name;
    } identifier;

    // Function call: id(argument1, argument2, ...)
    struct
    {
      char *name;
      ASTNode **arguments; // Array of expression nodes
      int arg_count;
    } func_call;

    // Assignment expression: left = right
    struct
    {
      ASTNode *left; // Usually an identifier node
      ASTNode *right;
    } assign_expr;

    // Return statement
    struct
    {
      ASTNode *expr; // NULL for void return
    } return_stmt;
  } data;
};

// Function prototypes for creating AST nodes
ASTNode *create_var_decl(int is_const, char *identifier, char *type_annotation, ASTNode *initializer);
ASTNode *create_print_stmt(ASTNode *expr);
ASTNode *create_prompt_stmt(ASTNode *expr);
ASTNode *create_if_stmt(ASTNode *condition, ASTNode *if_block, ASTNode *else_block);
ASTNode *create_while_stmt(ASTNode *condition, ASTNode *block);
ASTNode *create_for_stmt(char *iterator, ASTNode *start_expr, ASTNode *end_expr, ASTNode *block);
ASTNode *create_func_def(char *name, ASTNode **parameters, int param_count, char *return_type, ASTNode *body);
ASTNode *create_expr_stmt(ASTNode *expr);
ASTNode *create_block(ASTNode **statements, int stmt_count);
ASTNode *create_binary_expr(char *op, ASTNode *left, ASTNode *right);
ASTNode *create_unary_expr(char *op, ASTNode *operand);
ASTNode *create_literal(char *value);
ASTNode *create_identifier(char *name);
ASTNode *create_func_call(char *name, ASTNode **arguments, int arg_count);
ASTNode *create_assign_expr(ASTNode *left, ASTNode *right);
ASTNode *create_return_stmt(ASTNode *expr);

// Free an AST node (recursively)
void free_ast(ASTNode *node);

#endif // AST_H
