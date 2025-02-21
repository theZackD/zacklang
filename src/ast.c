#include "../include/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Memory allocation helper functions for robustness.
static void *xmalloc(size_t size)
{
  void *ptr = malloc(size);
  if (!ptr)
  {
    fprintf(stderr, "Memory allocation failed for %zu bytes\n", size);
    exit(EXIT_FAILURE);
  }
  return ptr;
}

static char *xstrdup(const char *s)
{
  char *ptr = strdup(s);
  if (!ptr)
  {
    fprintf(stderr, "Memory allocation failed in strdup\n");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

// Create a variable declaration node.
ASTNode *create_var_decl(int is_const, char *identifier, char *type_annotation,
                         ASTNode *initializer)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_VAR_DECL;
  node->data.var_decl.is_const = is_const;
  node->data.var_decl.identifier = xstrdup(identifier);
  node->data.var_decl.type_annotation = type_annotation ? xstrdup(type_annotation) : NULL;
  node->data.var_decl.initializer = initializer;
  return node;
}

// Create a print statement node.
ASTNode *create_print_stmt(ASTNode *expr)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_PRINT_STMT;
  node->data.print_stmt.expr = expr;
  return node;
}

// Create a prompt statement node.
ASTNode *create_prompt_stmt(ASTNode *expr)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_PROMPT_STMT;
  node->data.prompt_stmt.expr = expr;
  return node;
}

// Create an if statement node.
ASTNode *create_if_stmt(ASTNode *condition, ASTNode *if_block,
                        ASTNode **elif_conds, ASTNode **elif_blocks, int elif_count,
                        ASTNode *else_block)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_IF_STMT;
  node->data.if_stmt.condition = condition;
  node->data.if_stmt.if_block = if_block;
  node->data.if_stmt.elif_conds = elif_conds;
  node->data.if_stmt.elif_blocks = elif_blocks;
  node->data.if_stmt.elif_count = elif_count;
  node->data.if_stmt.else_block = else_block;
  return node;
}

// Create a while loop node.
ASTNode *create_while_stmt(ASTNode *condition, ASTNode *block)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_WHILE_STMT;
  node->data.while_stmt.condition = condition;
  node->data.while_stmt.block = block;
  return node;
}

// Create a for loop node (range-based).
ASTNode *create_for_stmt(char *iterator, ASTNode *start_expr, ASTNode *end_expr,
                         ASTNode *block)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_FOR_STMT;
  node->data.for_stmt.iterator = xstrdup(iterator);
  node->data.for_stmt.start_expr = start_expr;
  node->data.for_stmt.end_expr = end_expr;
  node->data.for_stmt.block = block;
  return node;
}

// Create a function definition node.
ASTNode *create_func_def(char *name, ASTNode **parameters, int param_count,
                         char *return_type, ASTNode *body, int is_comptime)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_FUNC_DEF;
  node->data.func_def.name = xstrdup(name);
  node->data.func_def.parameters = parameters;
  node->data.func_def.param_count = param_count;
  node->data.func_def.return_type = return_type ? xstrdup(return_type) : NULL;
  node->data.func_def.body = body;
  node->data.func_def.is_comptime = is_comptime;
  return node;
}

// Create an expression statement node.
ASTNode *create_expr_stmt(ASTNode *expr)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_EXPR_STMT;
  node->data.expr_stmt.expr = expr;
  return node;
}

// Create a block node, which represents a series of statements.
ASTNode *create_block(ASTNode **statements, int stmt_count)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_BLOCK;
  node->data.block.statements = statements;
  node->data.block.stmt_count = stmt_count;
  return node;
}

// Create a binary expression node.
ASTNode *create_binary_expr(char *op, ASTNode *left, ASTNode *right)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_BINARY_EXPR;
  node->data.binary_expr.op = xstrdup(op);
  node->data.binary_expr.left = left;
  node->data.binary_expr.right = right;
  return node;
}

// Create a unary expression node.
ASTNode *create_unary_expr(char *op, ASTNode *operand)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_UNARY_EXPR;
  node->data.unary_expr.op = xstrdup(op);
  node->data.unary_expr.operand = operand;
  return node;
}

// Create a literal node.
ASTNode *create_literal(const char *value)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_LITERAL;
  node->data.literal.value = xstrdup(value);
  return node;
}

// Create an identifier node.
ASTNode *create_identifier(char *name)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_IDENTIFIER;
  node->data.identifier.name = xstrdup(name);
  return node;
}

// Create a function call node.
ASTNode *create_func_call(char *name, ASTNode **arguments, int arg_count)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_FUNC_CALL;
  node->data.func_call.name = xstrdup(name);
  node->data.func_call.arguments = arguments;
  node->data.func_call.arg_count = arg_count;
  return node;
}

// Create an assignment expression node.
ASTNode *create_assign_expr(ASTNode *left, ASTNode *right)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_ASSIGN_EXPR;
  node->data.assign_expr.left = left;
  node->data.assign_expr.right = right;
  return node;
}

// Create a return statement node.
ASTNode *create_return_stmt(ASTNode *expr)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_RETURN_STMT;
  node->data.return_stmt.expr = expr;
  return node;
}

// Create an array literal node.
ASTNode *create_array_literal(ASTNode **elements, int element_count)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_ARRAY_LITERAL;
  node->data.array_literal.elements = elements;
  node->data.array_literal.element_count = element_count;
  return node;
}

// Create an array indexing node.
ASTNode *create_array_index(ASTNode *array, ASTNode *index)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_ARRAY_INDEX;
  node->data.array_index.array = array;
  node->data.array_index.index = index;
  return node;
}

// Create a break statement node.
ASTNode *create_break_stmt(void)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_BREAK_STMT;
  return node;
}

// Create a continue statement node.
ASTNode *create_continue_stmt(void)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_CONTINUE_STMT;
  return node;
}

// Create a switch statement node.
ASTNode *create_switch_stmt(ASTNode *expr, ASTNode **cases, int case_count, ASTNode *finally_block)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_SWITCH_STMT;
  node->data.switch_stmt.expr = expr;
  node->data.switch_stmt.cases = cases;
  node->data.switch_stmt.case_count = case_count;
  node->data.switch_stmt.finally_block = finally_block;
  return node;
}

// Create a case statement node.
ASTNode *create_case_stmt(ASTNode *expr, ASTNode *statement)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_CASE_STMT;
  node->data.case_stmt.expr = expr;
  node->data.case_stmt.statement = statement;
  return node;
}

// Create an f-string node.
ASTNode *create_fstring(ASTNode **parts, int part_count)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_FSTRING;
  node->data.fstring.parts = parts;
  node->data.fstring.part_count = part_count;
  return node;
}

// Create a string interpolation node.
ASTNode *create_string_interp(ASTNode *expr)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_STRING_INTERP;
  node->data.string_interp.expr = expr;
  return node;
}

// Create a struct definition node.
ASTNode *create_struct_def(char *name, char **field_names, char **field_types, int field_count)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_STRUCT_DEF;
  node->data.struct_def.name = xstrdup(name);

  // Allocate and copy field names and types.
  node->data.struct_def.field_names = (char **)xmalloc(field_count * sizeof(char *));
  node->data.struct_def.field_types = (char **)xmalloc(field_count * sizeof(char *));
  node->data.struct_def.field_count = field_count;

  for (int i = 0; i < field_count; i++)
  {
    node->data.struct_def.field_names[i] = xstrdup(field_names[i]);
    node->data.struct_def.field_types[i] = xstrdup(field_types[i]);
  }
  return node;
}

// Create a field access node.
ASTNode *create_field_access(ASTNode *struct_expr, char *field_name)
{
  ASTNode *node = (ASTNode *)xmalloc(sizeof(ASTNode));
  node->type = AST_FIELD_ACCESS;
  node->data.field_access.struct_expr = struct_expr;
  node->data.field_access.field_name = xstrdup(field_name);
  return node;
}

// Recursively free an AST node and all its children.
void free_ast(ASTNode *node)
{
  if (!node)
    return;
  switch (node->type)
  {
  case AST_VAR_DECL:
    free(node->data.var_decl.identifier);
    if (node->data.var_decl.type_annotation)
      free(node->data.var_decl.type_annotation);
    if (node->data.var_decl.initializer)
      free_ast(node->data.var_decl.initializer);
    break;
  case AST_PRINT_STMT:
    free_ast(node->data.print_stmt.expr);
    break;
  case AST_PROMPT_STMT:
    free_ast(node->data.prompt_stmt.expr);
    break;
  case AST_IF_STMT:
    free_ast(node->data.if_stmt.condition);
    free_ast(node->data.if_stmt.if_block);
    if (node->data.if_stmt.elif_count > 0)
    {
      for (int i = 0; i < node->data.if_stmt.elif_count; i++)
      {
        free_ast(node->data.if_stmt.elif_conds[i]);
        free_ast(node->data.if_stmt.elif_blocks[i]);
      }
      free(node->data.if_stmt.elif_conds);
      free(node->data.if_stmt.elif_blocks);
    }
    if (node->data.if_stmt.else_block)
      free_ast(node->data.if_stmt.else_block);
    break;
  case AST_WHILE_STMT:
    free_ast(node->data.while_stmt.condition);
    free_ast(node->data.while_stmt.block);
    break;
  case AST_FOR_STMT:
    free(node->data.for_stmt.iterator);
    free_ast(node->data.for_stmt.start_expr);
    free_ast(node->data.for_stmt.end_expr);
    free_ast(node->data.for_stmt.block);
    break;
  case AST_FUNC_DEF:
    free(node->data.func_def.name);
    for (int i = 0; i < node->data.func_def.param_count; i++)
      free_ast(node->data.func_def.parameters[i]);
    free(node->data.func_def.parameters);
    if (node->data.func_def.return_type)
      free(node->data.func_def.return_type);
    free_ast(node->data.func_def.body);
    break;
  case AST_EXPR_STMT:
    free_ast(node->data.expr_stmt.expr);
    break;
  case AST_BLOCK:
    for (int i = 0; i < node->data.block.stmt_count; i++)
      free_ast(node->data.block.statements[i]);
    free(node->data.block.statements);
    break;
  case AST_BINARY_EXPR:
    free(node->data.binary_expr.op);
    free_ast(node->data.binary_expr.left);
    free_ast(node->data.binary_expr.right);
    break;
  case AST_UNARY_EXPR:
    free(node->data.unary_expr.op);
    free_ast(node->data.unary_expr.operand);
    break;
  case AST_LITERAL:
    free(node->data.literal.value);
    break;
  case AST_IDENTIFIER:
    free(node->data.identifier.name);
    break;
  case AST_FUNC_CALL:
    free(node->data.func_call.name);
    for (int i = 0; i < node->data.func_call.arg_count; i++)
      free_ast(node->data.func_call.arguments[i]);
    free(node->data.func_call.arguments);
    break;
  case AST_ASSIGN_EXPR:
    free_ast(node->data.assign_expr.left);
    free_ast(node->data.assign_expr.right);
    break;
  case AST_RETURN_STMT:
    if (node->data.return_stmt.expr)
      free_ast(node->data.return_stmt.expr);
    break;
  case AST_ARRAY_LITERAL:
    for (int i = 0; i < node->data.array_literal.element_count; i++)
      free_ast(node->data.array_literal.elements[i]);
    free(node->data.array_literal.elements);
    break;
  case AST_ARRAY_INDEX:
    free_ast(node->data.array_index.array);
    free_ast(node->data.array_index.index);
    break;
  case AST_SWITCH_STMT:
    free_ast(node->data.switch_stmt.expr);
    for (int i = 0; i < node->data.switch_stmt.case_count; i++)
      free_ast(node->data.switch_stmt.cases[i]);
    free(node->data.switch_stmt.cases);
    if (node->data.switch_stmt.finally_block)
      free_ast(node->data.switch_stmt.finally_block);
    break;
  case AST_CASE_STMT:
    free_ast(node->data.case_stmt.expr);
    free_ast(node->data.case_stmt.statement);
    break;
  case AST_FSTRING:
    for (int i = 0; i < node->data.fstring.part_count; i++)
      free_ast(node->data.fstring.parts[i]);
    free(node->data.fstring.parts);
    break;
  case AST_STRING_INTERP:
    free_ast(node->data.string_interp.expr);
    break;
  case AST_STRUCT_DEF:
    free(node->data.struct_def.name);
    for (int i = 0; i < node->data.struct_def.field_count; i++)
    {
      free(node->data.struct_def.field_names[i]);
      free(node->data.struct_def.field_types[i]);
    }
    free(node->data.struct_def.field_names);
    free(node->data.struct_def.field_types);
    break;
  case AST_FIELD_ACCESS:
    free_ast(node->data.field_access.struct_expr);
    free(node->data.field_access.field_name);
    break;
  default:
    break;
  }
  free(node);
}
