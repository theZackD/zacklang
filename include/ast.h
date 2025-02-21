#ifndef AST_H
#define AST_H

// Enumeration for AST node types.
typedef enum
{
  AST_VAR_DECL,      // Variable declaration.
  AST_PRINT_STMT,    // Print statement.
  AST_PROMPT_STMT,   // Prompt statement.
  AST_IF_STMT,       // If statement (includes else).
  AST_WHILE_STMT,    // While loop.
  AST_FOR_STMT,      // For loop (range-based).
  AST_FUNC_DEF,      // Function definition.
  AST_EXPR_STMT,     // Expression used as a statement.
  AST_BLOCK,         // Block of statements.
  AST_BINARY_EXPR,   // Binary expression.
  AST_UNARY_EXPR,    // Unary expression.
  AST_LITERAL,       // Literal (number, string, etc.).
  AST_IDENTIFIER,    // Identifier.
  AST_FUNC_CALL,     // Function call.
  AST_ASSIGN_EXPR,   // Assignment expression.
  AST_RETURN_STMT,   // Return statement.
  AST_ARRAY_LITERAL, // Array literal (e.g. [1, 2, 3]).
  AST_ARRAY_INDEX,   // Array indexing expression.
  AST_BREAK_STMT,    // Break statement.
  AST_CONTINUE_STMT, // Continue statement.
  AST_SWITCH_STMT,   // Switch statement.
  AST_CASE_STMT,     // Case statement.
  AST_FSTRING,       // f-string literal.
  AST_STRING_INTERP, // String interpolation expression.
  AST_STRUCT_DEF,    // Struct definition.
  AST_FIELD_ACCESS   // Field access (struct.field).
} ASTNodeType;

// Forward declaration.
typedef struct ASTNode ASTNode;

// Definition for an AST node.
struct ASTNode
{
  ASTNodeType type;
  union
  {
    // Variable declaration: let [const] id [: type] = initializer;
    struct
    {
      int is_const;
      char *identifier;
      char *type_annotation; // Initially stored as a string.
      ASTNode *initializer;  // Expression node.
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
    struct
    {
      ASTNode *condition;
      ASTNode *if_block;
      ASTNode **elif_conds;
      ASTNode **elif_blocks;
      int elif_count;
      ASTNode *else_block;
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
      ASTNode **parameters; // Array of parameter nodes.
      int param_count;
      char *return_type;
      ASTNode *body;   // Block node.
      int is_comptime; // Flag for compile-time functions.
    } func_def;

    // Expression statement: expression;
    struct
    {
      ASTNode *expr;
    } expr_stmt;

    // Block: { statement1, statement2, ... }
    struct
    {
      ASTNode **statements;
      int stmt_count;
    } block;

    // Binary expression: left op right.
    struct
    {
      char *op;
      ASTNode *left;
      ASTNode *right;
    } binary_expr;

    // Unary expression: op operand.
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

    // Identifier: variable and function names.
    struct
    {
      char *name;
    } identifier;

    // Function call: id(argument1, argument2, ...)
    struct
    {
      char *name;
      ASTNode **arguments;
      int arg_count;
    } func_call;

    // Assignment expression: left = right.
    struct
    {
      ASTNode *left;
      ASTNode *right;
    } assign_expr;

    // Return statement.
    struct
    {
      ASTNode *expr; // May be NULL.
    } return_stmt;

    // Array literal: [expr1, expr2, ...].
    struct
    {
      ASTNode **elements;
      int element_count;
    } array_literal;

    // Array indexing: array[index].
    struct
    {
      ASTNode *array;
      ASTNode *index;
    } array_index;

    // Switch statement: switch (expr) { cases... finally... }
    struct
    {
      ASTNode *expr;
      ASTNode **cases;
      int case_count;
      ASTNode *finally_block;
    } switch_stmt;

    // Case statement: case expr: statement.
    struct
    {
      ASTNode *expr;
      ASTNode *statement;
    } case_stmt;

    // F-string: sequence of literals and interpolated expressions.
    struct
    {
      ASTNode **parts;
      int part_count;
    } fstring;

    // String interpolation: expression to be evaluated.
    struct
    {
      ASTNode *expr;
    } string_interp;

    // Struct definition.
    struct
    {
      char *name;
      char **field_names;
      char **field_types;
      int field_count;
    } struct_def;

    // Field access: struct.field.
    struct
    {
      ASTNode *struct_expr;
      char *field_name;
    } field_access;
  } data;
};

// Function prototypes for creating AST nodes.
ASTNode *create_var_decl(int is_const, char *identifier, char *type_annotation, ASTNode *initializer);
ASTNode *create_print_stmt(ASTNode *expr);
ASTNode *create_prompt_stmt(ASTNode *expr);
ASTNode *create_if_stmt(ASTNode *condition, ASTNode *if_block,
                        ASTNode **elif_conds, ASTNode **elif_blocks, int elif_count,
                        ASTNode *else_block);
ASTNode *create_while_stmt(ASTNode *condition, ASTNode *block);
ASTNode *create_for_stmt(char *iterator, ASTNode *start_expr, ASTNode *end_expr, ASTNode *block);
ASTNode *create_func_def(char *name, ASTNode **parameters, int param_count, char *return_type, ASTNode *body, int is_comptime);
ASTNode *create_expr_stmt(ASTNode *expr);
ASTNode *create_block(ASTNode **statements, int stmt_count);
ASTNode *create_binary_expr(char *op, ASTNode *left, ASTNode *right);
ASTNode *create_unary_expr(char *op, ASTNode *operand);
ASTNode *create_literal(const char *value);
ASTNode *create_identifier(char *name);
ASTNode *create_func_call(char *name, ASTNode **arguments, int arg_count);
ASTNode *create_assign_expr(ASTNode *left, ASTNode *right);
ASTNode *create_return_stmt(ASTNode *expr);
ASTNode *create_array_literal(ASTNode **elements, int element_count);
ASTNode *create_array_index(ASTNode *array, ASTNode *index);
ASTNode *create_break_stmt(void);
ASTNode *create_continue_stmt(void);
ASTNode *create_switch_stmt(ASTNode *expr, ASTNode **cases, int case_count, ASTNode *finally_block);
ASTNode *create_case_stmt(ASTNode *expr, ASTNode *statement);
ASTNode *create_fstring(ASTNode **parts, int part_count);
ASTNode *create_string_interp(ASTNode *expr);
ASTNode *create_struct_def(char *name, char **field_names, char **field_types, int field_count);
ASTNode *create_field_access(ASTNode *struct_expr, char *field_name);

// Free an AST node (recursively).
void free_ast(ASTNode *node);

#endif // AST_H
