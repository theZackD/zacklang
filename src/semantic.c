#include "../include/semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Forward declarations for helper functions
static void check_expression_type(ASTNode *node, SymbolTable *table);
static const char *get_expression_type(ASTNode *node, SymbolTable *table);

// Forward declaration of test exit function
extern void test_exit(int status);

// Helper function to report errors
static void semantic_error(const char *message, ...)
{
  va_list args;
  va_start(args, message);
  vfprintf(stderr, message, args);
  va_end(args);
  test_exit(1);
}

// Debug function to print AST node type
static void print_node_type(ASTNode *node)
{
  if (!node)
  {
    printf("DEBUG: Node is NULL\n");
    return;
  }
  printf("DEBUG: Processing node type %d\n", node->type);
}

// Helper function to get function parameter count
static int get_func_param_count(ASTNode *func_def)
{
  if (func_def->type != AST_FUNC_DEF)
  {
    semantic_error("Internal Error: Expected function definition node\n");
  }
  return func_def->data.func_def.param_count;
}

// Recursively walk the AST to perform semantic checks
void semantic_visit(ASTNode *node, SymbolTable *table)
{
  if (!node)
    return;

  print_node_type(node);

  switch (node->type)
  {
  case AST_VAR_DECL:
  {
    printf("DEBUG: Checking variable declaration for '%s'\n",
           node->data.var_decl.identifier);

    // Check for duplicate declarations in the current scope only
    for (int i = 0; i < table->count; i++)
    {
      if (strcmp(table->symbols[i]->name, node->data.var_decl.identifier) == 0)
      {
        semantic_error("Semantic Error: Duplicate declaration of '%s' in current scope\n",
                       node->data.var_decl.identifier);
      }
    }

    // Require a type annotation
    if (!node->data.var_decl.type_annotation)
    {
      semantic_error("Semantic Error: Missing type annotation for variable '%s'\n",
                     node->data.var_decl.identifier);
    }

    // Check initializer expression
    if (node->data.var_decl.initializer)
    {
      printf("DEBUG: Checking initializer for '%s'\n",
             node->data.var_decl.identifier);
      semantic_visit(node->data.var_decl.initializer, table);

      // Type check the initializer
      const char *init_type = get_expression_type(node->data.var_decl.initializer, table);
      printf("DEBUG: Initializer type is '%s'\n", init_type);

      if (strcmp(init_type, node->data.var_decl.type_annotation) != 0)
      {
        semantic_error("Semantic Error: Type mismatch in initialization of '%s'. Expected %s, got %s\n",
                       node->data.var_decl.identifier,
                       node->data.var_decl.type_annotation,
                       init_type);
      }
    }

    // Add the symbol to the current symbol table
    printf("DEBUG: Adding symbol '%s' with type '%s'\n",
           node->data.var_decl.identifier,
           node->data.var_decl.type_annotation);
    add_symbol(table, node->data.var_decl.identifier,
               node->data.var_decl.type_annotation);
    break;
  }

  case AST_IDENTIFIER:
  {
    // Check if the identifier has been declared
    Symbol *symbol = lookup_symbol(table, node->data.identifier.name);
    if (!symbol)
    {
      semantic_error("Semantic Error: Undeclared identifier '%s'\n",
                     node->data.identifier.name);
    }
    break;
  }

  case AST_FUNC_DEF:
  {
    // Check for duplicate function declarations
    for (int i = 0; i < table->count; i++)
    {
      if (strcmp(table->symbols[i]->name, node->data.func_def.name) == 0)
      {
        semantic_error("Semantic Error: Duplicate function declaration '%s' in current scope\n",
                       node->data.func_def.name);
      }
    }

    // Add function to symbol table before processing body (to allow recursion)
    const char *return_type = node->data.func_def.return_type
                                  ? node->data.func_def.return_type
                                  : "void";
    add_symbol_with_node(table, node->data.func_def.name, return_type, node);

    // Create new scope for function body
    SymbolTable *func_scope = create_symbol_table(table);

    // Add parameters to function scope
    for (int i = 0; i < node->data.func_def.param_count; i++)
    {
      ASTNode *param = node->data.func_def.parameters[i];
      if (param->type != AST_VAR_DECL)
      {
        semantic_error("Semantic Error: Invalid parameter in function '%s'\n",
                       node->data.func_def.name);
      }
      add_symbol(func_scope, param->data.var_decl.identifier,
                 param->data.var_decl.type_annotation);
    }

    // Process function body
    semantic_visit(node->data.func_def.body, func_scope);
    destroy_symbol_table(func_scope);
    break;
  }

  case AST_FUNC_CALL:
  {
    // Check if function exists
    Symbol *func = lookup_symbol(table, node->data.func_call.name);
    if (!func)
    {
      semantic_error("Semantic Error: Call to undefined function '%s'\n",
                     node->data.func_call.name);
    }

    // Check parameter count
    if (func->node && func->node->type == AST_FUNC_DEF)
    {
      int expected_params = func->node->data.func_def.param_count;
      if (node->data.func_call.arg_count != expected_params)
      {
        semantic_error("Semantic Error: Function '%s' expects %d arguments, but got %d\n",
                       node->data.func_call.name,
                       expected_params,
                       node->data.func_call.arg_count);
      }
    }

    // Check arguments
    for (int i = 0; i < node->data.func_call.arg_count; i++)
    {
      semantic_visit(node->data.func_call.arguments[i], table);
    }
    break;
  }

  case AST_BLOCK:
  {
    // Create new scope for block
    SymbolTable *block_scope = create_symbol_table(table);
    for (int i = 0; i < node->data.block.stmt_count; i++)
    {
      semantic_visit(node->data.block.statements[i], block_scope);
    }
    destroy_symbol_table(block_scope);
    break;
  }

  case AST_BINARY_EXPR:
  {
    semantic_visit(node->data.binary_expr.left, table);
    semantic_visit(node->data.binary_expr.right, table);
    check_expression_type(node, table);
    break;
  }

  case AST_UNARY_EXPR:
  {
    semantic_visit(node->data.unary_expr.operand, table);
    check_expression_type(node, table);
    break;
  }

  case AST_IF_STMT:
  {
    semantic_visit(node->data.if_stmt.condition, table);
    semantic_visit(node->data.if_stmt.if_block, table);
    if (node->data.if_stmt.else_block)
    {
      semantic_visit(node->data.if_stmt.else_block, table);
    }
    break;
  }

  case AST_WHILE_STMT:
  {
    semantic_visit(node->data.while_stmt.condition, table);
    semantic_visit(node->data.while_stmt.block, table);
    break;
  }

  case AST_FOR_STMT:
  {
    // Create new scope for the loop
    SymbolTable *loop_scope = create_symbol_table(table);

    // Add iterator variable to scope
    add_symbol(loop_scope, node->data.for_stmt.iterator, "i32");

    semantic_visit(node->data.for_stmt.start_expr, table);
    semantic_visit(node->data.for_stmt.end_expr, table);
    semantic_visit(node->data.for_stmt.block, loop_scope);

    destroy_symbol_table(loop_scope);
    break;
  }

  case AST_ASSIGN_EXPR:
  {
    semantic_visit(node->data.assign_expr.left, table);
    semantic_visit(node->data.assign_expr.right, table);

    // Check type compatibility
    const char *left_type = get_expression_type(node->data.assign_expr.left, table);
    const char *right_type = get_expression_type(node->data.assign_expr.right, table);

    if (strcmp(left_type, right_type) != 0)
    {
      semantic_error("Semantic Error: Type mismatch in assignment. Expected %s, got %s\n",
                     left_type, right_type);
    }
    break;
  }

  case AST_EXPR_STMT:
    semantic_visit(node->data.expr_stmt.expr, table);
    break;

  case AST_PRINT_STMT:
    semantic_visit(node->data.print_stmt.expr, table);
    break;

  case AST_PROMPT_STMT:
    semantic_visit(node->data.prompt_stmt.expr, table);
    break;

  case AST_LITERAL:
    // Nothing to check for literals
    break;

  default:
    // Unhandled node type
    semantic_error("Semantic Error: Unhandled AST node type %d\n", node->type);
  }
}

// Helper function to get the type of an expression
static const char *get_expression_type(ASTNode *node, SymbolTable *table)
{
  if (!node)
  {
    return "unknown";
  }

  switch (node->type)
  {
  case AST_IDENTIFIER:
  {
    Symbol *symbol = lookup_symbol(table, node->data.identifier.name);
    return symbol ? symbol->type : "unknown";
  }

  case AST_LITERAL:
  {
    printf("DEBUG: Checking literal value '%s'\n", node->data.literal.value);
    char first_char = node->data.literal.value[0];
    if (first_char == '"')
      return "string";
    if (first_char == 't' || first_char == 'f')
      return "bool";
    if (strchr(node->data.literal.value, '.'))
      return "f64";
    return "i32";
  }

  case AST_FUNC_CALL:
  {
    Symbol *func = lookup_symbol(table, node->data.func_call.name);
    return func ? func->type : "unknown";
  }

  case AST_BINARY_EXPR:
  {
    const char *left_type = get_expression_type(node->data.binary_expr.left, table);
    const char *right_type = get_expression_type(node->data.binary_expr.right, table);

    // For now, simple type rules: if both operands are the same type, that's the result type
    if (strcmp(left_type, right_type) == 0)
      return left_type;

    // If one is f64 and other is i32, result is f64
    if ((strcmp(left_type, "f64") == 0 && strcmp(right_type, "i32") == 0) ||
        (strcmp(left_type, "i32") == 0 && strcmp(right_type, "f64") == 0))
    {
      return "f64";
    }

    return "unknown";
  }

  case AST_UNARY_EXPR:
  {
    return get_expression_type(node->data.unary_expr.operand, table);
  }

  default:
    return "unknown";
  }
}

// Helper function to check if an expression's type is valid
static void check_expression_type(ASTNode *node, SymbolTable *table)
{
  const char *type = get_expression_type(node, table);
  if (strcmp(type, "unknown") == 0)
  {
    semantic_error("Semantic Error: Invalid type in expression\n");
  }
}

// Entry point for semantic analysis
void semantic_analysis(ASTNode *root)
{
  printf("DEBUG: Starting semantic analysis\n");
  SymbolTable *global = create_symbol_table(NULL);
  semantic_visit(root, global);
  destroy_symbol_table(global);
  printf("DEBUG: Completed semantic analysis\n");
}
