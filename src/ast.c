#include "../include/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Create a variable declaration node.
ASTNode *create_var_decl(int is_const, char *identifier, char *type_annotation,
                         ASTNode *initializer)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_VAR_DECL;
  node->data.var_decl.is_const = is_const;
  node->data.var_decl.identifier = strdup(identifier);
  node->data.var_decl.type_annotation =
      type_annotation ? strdup(type_annotation) : NULL;
  node->data.var_decl.initializer = initializer;
  return node;
}

// Create a print statement node.
ASTNode *create_print_stmt(ASTNode *expr)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_PRINT_STMT;
  node->data.print_stmt.expr = expr;
  return node;
}

// Create a prompt statement node.
ASTNode *create_prompt_stmt(ASTNode *expr)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_PROMPT_STMT;
  node->data.prompt_stmt.expr = expr;
  return node;
}

// Create an if statement node.
ASTNode *create_if_stmt(ASTNode *condition, ASTNode *if_block,
                        ASTNode *else_block)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_IF_STMT;
  node->data.if_stmt.condition = condition;
  node->data.if_stmt.if_block = if_block;
  node->data.if_stmt.else_block = else_block;
  return node;
}

// Create a while loop node.
ASTNode *create_while_stmt(ASTNode *condition, ASTNode *block)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_WHILE_STMT;
  node->data.while_stmt.condition = condition;
  node->data.while_stmt.block = block;

  return node;
}

// Create a for loop node (range-based).
ASTNode *create_for_stmt(char *iterator, ASTNode *start_expr, ASTNode *end_expr,
                         ASTNode *block)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_FOR_STMT;
  node->data.for_stmt.iterator = strdup(iterator);
  node->data.for_stmt.start_expr = start_expr;
  node->data.for_stmt.end_expr = end_expr;
  node->data.for_stmt.block = block;

  return node;
}

// Create a function definition node.
ASTNode *create_func_def(char *name, ASTNode **parameters, int param_count,
                         char *return_type, ASTNode *body)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_FUNC_DEF;
  node->data.func_def.name = strdup(name);
  node->data.func_def.parameters = parameters;
  node->data.func_def.param_count = param_count;
  node->data.func_def.return_type = return_type ? strdup(return_type) : NULL;
  node->data.func_def.body = body;
  return node;
}

// Create an expression statement node.
ASTNode *create_expr_stmt(ASTNode *expr)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_EXPR_STMT;
  node->data.expr_stmt.expr = expr;

  return node;
}

// Create a block node, which represents a series of statements.
ASTNode *create_block(ASTNode **statements, int stmt_count)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_BLOCK;
  node->data.block.statements = statements;
  node->data.block.stmt_count = stmt_count;

  return node;
}

// Create a binary expression node.
ASTNode *create_binary_expr(char *op, ASTNode *left, ASTNode *right)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_BINARY_EXPR;
  node->data.binary_expr.op = strdup(op);
  node->data.binary_expr.left = left;
  node->data.binary_expr.right = right;

  return node;
}

// Create a unary expression node.
ASTNode *create_unary_expr(char *op, ASTNode *operand)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_UNARY_EXPR;
  node->data.unary_expr.op = strdup(op);
  node->data.unary_expr.operand = operand;

  return node;
}

// Create a literal node.
ASTNode *create_literal(char *value)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_LITERAL;
  node->data.literal.value = strdup(value);

  return node;
}

// Create an identifier node.
ASTNode *create_identifier(char *name)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_IDENTIFIER;
  node->data.identifier.name = strdup(name);

  return node;
}

// Create a function call node.
ASTNode *create_func_call(char *name, ASTNode **arguments, int arg_count)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_FUNC_CALL;
  node->data.func_call.name = strdup(name);
  node->data.func_call.arguments = arguments;
  node->data.func_call.arg_count = arg_count;

  return node;
}

// Create an assignment expression node.
ASTNode *create_assign_expr(ASTNode *left, ASTNode *right)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_ASSIGN_EXPR;
  node->data.assign_expr.left = left;
  node->data.assign_expr.right = right;

  return node;
}

// Create a return statement node.
ASTNode *create_return_stmt(ASTNode *expr)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = AST_RETURN_STMT;
  node->data.return_stmt.expr = expr;
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
    if (node->data.var_decl.type_annotation != NULL)
    {
      free(node->data.var_decl.type_annotation);
    }
    if (node->data.var_decl.initializer != NULL)
    {
      free_ast(node->data.var_decl.initializer);
    }
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
    if (node->data.if_stmt.else_block != NULL)
    {
      free_ast(node->data.if_stmt.else_block);
    }
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
    {
      free_ast(node->data.func_def.parameters[i]);
    }
    free(node->data.func_def.parameters);
    if (node->data.func_def.return_type != NULL)
    {
      free(node->data.func_def.return_type);
    }
    free_ast(node->data.func_def.body);
    break;
  case AST_EXPR_STMT:
    free_ast(node->data.expr_stmt.expr);
    break; // Missing break; added here.
  case AST_BLOCK:
    for (int i = 0; i < node->data.block.stmt_count; i++)
    {
      free_ast(node->data.block.statements[i]);
    }
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
    break; // Missing break; added here.
  case AST_FUNC_CALL:
    free(node->data.func_call.name);
    for (int i = 0; i < node->data.func_call.arg_count; i++)
    {
      free_ast(node->data.func_call.arguments[i]);
    }
    free(node->data.func_call.arguments);
    break;
  case AST_ASSIGN_EXPR:
    free_ast(node->data.assign_expr.left);
    free_ast(node->data.assign_expr.right);
    break;
  case AST_RETURN_STMT:
    if (node->data.return_stmt.expr)
    {
      free_ast(node->data.return_stmt.expr);
    }
    break;
  default:
    break;
  }
  free(node);
}
