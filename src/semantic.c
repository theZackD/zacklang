#include "../include/semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Forward declarations for helper functions.
static void check_expression_type(ASTNode *node, SymbolTable *table);
static const char *get_expression_type(ASTNode *node, SymbolTable *table);
static int is_numeric_type(const char *type);
static int is_primitive_type(const char *type);

// External function for test exits.
extern void test_exit(int status);

// Global static variables for current function context and loop depth.
static const char *current_function_return_type = NULL;
static int loop_depth = 0;

// Helper: returns true if the type is numeric.
static int is_numeric_type(const char *type)
{
  return strcmp(type, "i32") == 0 || strcmp(type, "i64") == 0 ||
         strcmp(type, "f32") == 0 || strcmp(type, "f64") == 0;
}

// Helper: Report a semantic error and exit.
static void semantic_error(const char *message, ...)
{
  va_list args;
  va_start(args, message);
  vfprintf(stderr, message, args);
  va_end(args);
  test_exit(1);
}

// Debug helper: print the AST node type (for tracing).
static void print_node_type(ASTNode *node)
{
  if (!node)
  {
    printf("DEBUG: Node is NULL\n");
    return;
  }
  printf("DEBUG: Processing node type %d\n", node->type);
}

// Helper: Get function parameter count (expects a function definition node).
static int get_func_param_count(ASTNode *func_def)
{
  if (func_def->type != AST_FUNC_DEF)
  {
    semantic_error("Internal Error: Expected function definition node\n");
  }
  return func_def->data.func_def.param_count;
}

// Recursively visit the AST to perform semantic checks.
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

    // Check for duplicate declarations in the current scope.
    for (int i = 0; i < table->count; i++)
    {
      if (strcmp(table->symbols[i]->name, node->data.var_decl.identifier) == 0)
      {
        semantic_error("Semantic Error: Duplicate declaration of '%s' in current scope\n",
                       node->data.var_decl.identifier);
      }
    }

    // Require a type annotation.
    if (!node->data.var_decl.type_annotation)
    {
      semantic_error("Semantic Error: Missing type annotation for variable '%s'\n",
                     node->data.var_decl.identifier);
    }

    // Check initializer, if present.
    if (node->data.var_decl.initializer)
    {
      printf("DEBUG: Checking initializer for '%s'\n",
             node->data.var_decl.identifier);
      semantic_visit(node->data.var_decl.initializer, table);
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

    // Add the variable to the current symbol table.
    printf("DEBUG: Adding symbol '%s' with type '%s'\n",
           node->data.var_decl.identifier,
           node->data.var_decl.type_annotation);
    add_symbol(table, node->data.var_decl.identifier,
               node->data.var_decl.type_annotation);
    break;
  }

  case AST_IDENTIFIER:
  {
    // Ensure the identifier was declared.
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
    // Save previous return type.
    const char *prev_return_type = current_function_return_type;

    // Set current function return type.
    current_function_return_type = node->data.func_def.return_type
                                       ? node->data.func_def.return_type
                                       : "void";

    // Check for duplicate function declarations.
    for (int i = 0; i < table->count; i++)
    {
      if (strcmp(table->symbols[i]->name, node->data.func_def.name) == 0)
      {
        semantic_error("Semantic Error: Duplicate function declaration '%s' in current scope\n",
                       node->data.func_def.name);
      }
    }

    // Add function to symbol table before analyzing its body.
    add_symbol_with_node(table, node->data.func_def.name,
                         current_function_return_type, node);

    // Create a new scope for the function body.
    SymbolTable *func_scope = create_symbol_table(table);

    // Add function parameters to the new scope.
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

    // Analyze the function body.
    semantic_visit(node->data.func_def.body, func_scope);
    destroy_symbol_table(func_scope);

    // Restore the previous function return type.
    current_function_return_type = prev_return_type;
    break;
  }

  case AST_FUNC_CALL:
  {
    // Check if the function exists.
    Symbol *func = lookup_symbol(table, node->data.func_call.name);
    if (!func)
    {
      semantic_error("Semantic Error: Call to undefined function '%s'\n",
                     node->data.func_call.name);
    }
    // If available, check parameter count and argument types.
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
      for (int i = 0; i < node->data.func_call.arg_count; i++)
      {
        semantic_visit(node->data.func_call.arguments[i], table);
        const char *arg_type = get_expression_type(node->data.func_call.arguments[i], table);
        const char *param_type = func->node->data.func_def.parameters[i]->data.var_decl.type_annotation;
        if (strcmp(arg_type, param_type) != 0)
        {
          semantic_error("Semantic Error: Argument %d of call to '%s' has wrong type. Expected %s, got %s\n",
                         i + 1, node->data.func_call.name, param_type, arg_type);
        }
      }
    }
    break;
  }

  case AST_BLOCK:
  {
    // Create a new scope for the block.
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
    // Analyze the main if condition and block.
    semantic_visit(node->data.if_stmt.condition, table);
    semantic_visit(node->data.if_stmt.if_block, table);
    // Analyze each elif branch.
    for (int i = 0; i < node->data.if_stmt.elif_count; i++)
    {
      semantic_visit(node->data.if_stmt.elif_conds[i], table);
      semantic_visit(node->data.if_stmt.elif_blocks[i], table);
      const char *cond_type = get_expression_type(node->data.if_stmt.elif_conds[i], table);
      if (strcmp(cond_type, "bool") != 0)
      {
        semantic_error("Semantic Error: Elif condition must be boolean, got %s\n", cond_type);
      }
    }
    // Analyze the else block if present.
    if (node->data.if_stmt.else_block)
    {
      semantic_visit(node->data.if_stmt.else_block, table);
    }
    // Verify that the main if condition is boolean.
    const char *main_cond_type = get_expression_type(node->data.if_stmt.condition, table);
    if (strcmp(main_cond_type, "bool") != 0)
    {
      semantic_error("Semantic Error: If condition must be boolean, got %s\n", main_cond_type);
    }
    break;
  }

  case AST_WHILE_STMT:
  {
    loop_depth++;
    semantic_visit(node->data.while_stmt.condition, table);
    semantic_visit(node->data.while_stmt.block, table);
    loop_depth--;
    break;
  }

  case AST_FOR_STMT:
  {
    loop_depth++;
    // Create a new scope for the loop.
    SymbolTable *loop_scope = create_symbol_table(table);
    // Add the iterator variable (using "i32" as a placeholder type).
    add_symbol(loop_scope, node->data.for_stmt.iterator, "i32");
    semantic_visit(node->data.for_stmt.start_expr, table);
    semantic_visit(node->data.for_stmt.end_expr, table);
    semantic_visit(node->data.for_stmt.block, loop_scope);
    destroy_symbol_table(loop_scope);
    loop_depth--;
    break;
  }

  case AST_ASSIGN_EXPR:
  {
    semantic_visit(node->data.assign_expr.left, table);
    semantic_visit(node->data.assign_expr.right, table);
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
    // Literals need no further checking.
    break;

  case AST_RETURN_STMT:
  {
    if (!current_function_return_type)
    {
      semantic_error("Semantic Error: Return statement outside of function\n");
    }
    if (node->data.return_stmt.expr)
    {
      semantic_visit(node->data.return_stmt.expr, table);
      const char *expr_type = get_expression_type(node->data.return_stmt.expr, table);
      if (strcmp(current_function_return_type, expr_type) != 0)
      {
        semantic_error("Semantic Error: Return type mismatch. Expected %s, got %s\n",
                       current_function_return_type, expr_type);
      }
    }
    else if (strcmp(current_function_return_type, "void") != 0)
    {
      semantic_error("Semantic Error: Expected return value in non-void function\n");
    }
    break;
  }

  case AST_ARRAY_LITERAL:
  {
    if (node->data.array_literal.element_count > 0)
    {
      semantic_visit(node->data.array_literal.elements[0], table);
      const char *first_type = get_expression_type(node->data.array_literal.elements[0], table);
      for (int i = 1; i < node->data.array_literal.element_count; i++)
      {
        semantic_visit(node->data.array_literal.elements[i], table);
        const char *curr_type = get_expression_type(node->data.array_literal.elements[i], table);
        if (strcmp(first_type, curr_type) != 0)
        {
          semantic_error("Semantic Error: Array literal contains mixed types: %s and %s\n",
                         first_type, curr_type);
        }
      }
    }
    break;
  }

  case AST_ARRAY_INDEX:
  {
    semantic_visit(node->data.array_index.array, table);
    semantic_visit(node->data.array_index.index, table);
    const char *array_type = get_expression_type(node->data.array_index.array, table);
    if (strstr(array_type, "[]") == NULL)
    {
      semantic_error("Semantic Error: Cannot index non-array type %s\n", array_type);
    }
    const char *index_type = get_expression_type(node->data.array_index.index, table);
    if (strcmp(index_type, "i32") != 0 && strcmp(index_type, "i64") != 0)
    {
      semantic_error("Semantic Error: Array index must be integer type, got %s\n", index_type);
    }
    break;
  }

  case AST_BREAK_STMT:
  {
    if (loop_depth == 0)
    {
      semantic_error("Semantic Error: Break statement outside of loop\n");
    }
    break;
  }

  case AST_CONTINUE_STMT:
  {
    if (loop_depth == 0)
    {
      semantic_error("Semantic Error: Continue statement outside of loop\n");
    }
    break;
  }

  case AST_SWITCH_STMT:
  {
    // Check the switch expression.
    semantic_visit(node->data.switch_stmt.expr, table);
    Symbol *switch_expr_sym = NULL;
    if (node->data.switch_stmt.expr->type == AST_IDENTIFIER)
    {
      switch_expr_sym = lookup_symbol(table, node->data.switch_stmt.expr->data.identifier.name);
      if (!switch_expr_sym)
      {
        semantic_error("Semantic Error: Undefined variable in switch expression\n");
      }
    }

    // Create a new scope for the switch.
    SymbolTable *switch_scope = create_symbol_table(table);
    for (int i = 0; i < node->data.switch_stmt.case_count; i++)
    {
      ASTNode *case_node = node->data.switch_stmt.cases[i];
      if (case_node->type != AST_CASE_STMT)
      {
        semantic_error("Semantic Error: Expected case statement in switch\n");
      }
      semantic_visit(case_node->data.case_stmt.expr, switch_scope);
      // (Basic type check for literal cases; can be extended.)
      if (switch_expr_sym && case_node->data.case_stmt.expr->type == AST_LITERAL)
      {
        if (strchr(case_node->data.case_stmt.expr->data.literal.value, '"') != NULL &&
            switch_expr_sym->type && strcmp(switch_expr_sym->type, "string") != 0)
        {
          semantic_error("Semantic Error: Case expression type does not match switch expression type\n");
        }
      }
      semantic_visit(case_node->data.case_stmt.statement, switch_scope);
    }
    if (node->data.switch_stmt.finally_block)
    {
      semantic_visit(node->data.switch_stmt.finally_block, switch_scope);
    }
    destroy_symbol_table(switch_scope);
    break;
  }

  case AST_CASE_STMT:
    // Case statements should only occur within a switch.
    semantic_error("Semantic Error: Case statement outside of switch\n");
    break;

  case AST_FSTRING:
  {
    for (int i = 0; i < node->data.fstring.part_count; i++)
    {
      ASTNode *part = node->data.fstring.parts[i];
      semantic_visit(part, table);
      if (part->type != AST_LITERAL && part->type != AST_STRING_INTERP)
      {
        semantic_error("Semantic Error: Invalid f-string part\n");
      }
    }
    break;
  }

  case AST_STRING_INTERP:
  {
    semantic_visit(node->data.string_interp.expr, table);
    break;
  }

  case AST_STRUCT_DEF:
  {
    // Check for duplicate struct definitions.
    Symbol *existing = lookup_symbol(table, node->data.struct_def.name);
    if (existing)
    {
      semantic_error("Semantic Error: Duplicate definition of struct '%s'\n",
                     node->data.struct_def.name);
    }
    // Check for duplicate field names.
    for (int i = 0; i < node->data.struct_def.field_count; i++)
    {
      for (int j = i + 1; j < node->data.struct_def.field_count; j++)
      {
        if (strcmp(node->data.struct_def.field_names[i],
                   node->data.struct_def.field_names[j]) == 0)
        {
          semantic_error("Semantic Error: Duplicate field name '%s' in struct '%s'\n",
                         node->data.struct_def.field_names[i],
                         node->data.struct_def.name);
        }
      }
    }
    // Verify each field type.
    for (int i = 0; i < node->data.struct_def.field_count; i++)
    {
      const char *field_type = node->data.struct_def.field_types[i];
      if (strncmp(field_type, "struct ", 7) == 0)
      {
        Symbol *ref_struct = lookup_symbol(table, field_type + 7);
        if (!ref_struct || !ref_struct->node || ref_struct->node->type != AST_STRUCT_DEF)
        {
          semantic_error("Semantic Error: Field '%s' references undefined struct type '%s'\n",
                         node->data.struct_def.field_names[i], field_type + 7);
        }
      }
      else if (!is_primitive_type(field_type))
      {
        semantic_error("Semantic Error: Invalid type '%s' for field '%s' in struct '%s'\n",
                       field_type, node->data.struct_def.field_names[i],
                       node->data.struct_def.name);
      }
    }
    // Add the struct to the symbol table.
    char type_name[256];
    snprintf(type_name, sizeof(type_name), "struct %s", node->data.struct_def.name);
    add_symbol_with_node(table, node->data.struct_def.name, type_name, node);
    break;
  }

  case AST_FIELD_ACCESS:
  {
    semantic_visit(node->data.field_access.struct_expr, table);
    const char *struct_type = get_expression_type(node->data.field_access.struct_expr, table);
    if (strcmp(struct_type, "unknown") == 0)
    {
      semantic_error("Semantic Error: Invalid field access\n");
    }
    break;
  }

  default:
    semantic_error("Semantic Error: Unhandled AST node type %d\n", node->type);
  }
}

// Helper: Determine the type of an expression.
static const char *get_expression_type(ASTNode *node, SymbolTable *table)
{
  if (!node)
    return "unknown";

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
    if (strcmp(node->data.binary_expr.op, "**") == 0)
    {
      if (!is_numeric_type(left_type) || !is_numeric_type(right_type))
      {
        semantic_error("Semantic Error: Power operator requires numeric operands, got %s and %s\n",
                       left_type, right_type);
      }
      return (strcmp(left_type, "f64") == 0 || strcmp(right_type, "f64") == 0) ? "f64" : "i32";
    }
    if (strcmp(node->data.binary_expr.op, "==") == 0 ||
        strcmp(node->data.binary_expr.op, "!=") == 0 ||
        strcmp(node->data.binary_expr.op, "<") == 0 ||
        strcmp(node->data.binary_expr.op, "<=") == 0 ||
        strcmp(node->data.binary_expr.op, ">") == 0 ||
        strcmp(node->data.binary_expr.op, ">=") == 0)
    {
      if (strcmp(left_type, right_type) != 0)
      {
        semantic_error("Semantic Error: Comparison operands must be of the same type, got %s and %s\n",
                       left_type, right_type);
      }
      return "bool";
    }
    if (strcmp(left_type, right_type) == 0)
      return left_type;
    if ((strcmp(left_type, "f64") == 0 && strcmp(right_type, "i32") == 0) ||
        (strcmp(left_type, "i32") == 0 && strcmp(right_type, "f64") == 0))
      return "f64";
    return "unknown";
  }
  case AST_UNARY_EXPR:
    return get_expression_type(node->data.unary_expr.operand, table);
  case AST_ARRAY_LITERAL:
  {
    if (node->data.array_literal.element_count > 0)
    {
      const char *elem_type = get_expression_type(node->data.array_literal.elements[0], table);
      static char array_type[64];
      snprintf(array_type, sizeof(array_type), "%s[]", elem_type);
      return array_type;
    }
    return "unknown[]";
  }
  case AST_ARRAY_INDEX:
  {
    const char *array_type = get_expression_type(node->data.array_index.array, table);
    static char elem_type[64];
    size_t len = strlen(array_type);
    if (len > 2)
    {
      strncpy(elem_type, array_type, len - 2);
      elem_type[len - 2] = '\0';
      return elem_type;
    }
    return "unknown";
  }
  case AST_FIELD_ACCESS:
  {
    const char *struct_type = get_expression_type(node->data.field_access.struct_expr, table);
    if (strncmp(struct_type, "struct ", 7) != 0)
    {
      semantic_error("Semantic Error: Cannot access field '%s' of non-struct type '%s'\n",
                     node->data.field_access.field_name, struct_type);
    }
    Symbol *struct_sym = lookup_symbol(table, struct_type + 7); // Skip "struct " prefix.
    if (!struct_sym || !struct_sym->node || struct_sym->node->type != AST_STRUCT_DEF)
    {
      semantic_error("Semantic Error: Undefined struct type '%s'\n", struct_type + 7);
    }
    ASTNode *struct_def = struct_sym->node;
    for (int i = 0; i < struct_def->data.struct_def.field_count; i++)
    {
      if (strcmp(struct_def->data.struct_def.field_names[i],
                 node->data.field_access.field_name) == 0)
      {
        return struct_def->data.struct_def.field_types[i];
      }
    }
    semantic_error("Semantic Error: Struct '%s' has no field named '%s'\n",
                   struct_type + 7, node->data.field_access.field_name);
    return "unknown";
  }
  default:
    return "unknown";
  }
}

// Helper: Check that an expression's type is not "unknown".
static void check_expression_type(ASTNode *node, SymbolTable *table)
{
  const char *type = get_expression_type(node, table);
  if (strcmp(type, "unknown") == 0)
  {
    semantic_error("Semantic Error: Invalid type in expression\n");
  }
}

// Entry point: perform semantic analysis starting from the root AST node.
void semantic_analysis(ASTNode *root)
{
  printf("DEBUG: Starting semantic analysis\n");
  SymbolTable *global = create_symbol_table(NULL);
  semantic_visit(root, global);
  destroy_symbol_table(global);
  printf("DEBUG: Completed semantic analysis\n");
}

// Helper: Check if a type is one of the recognized primitive types.
static int is_primitive_type(const char *type)
{
  return strcmp(type, "i32") == 0 ||
         strcmp(type, "i64") == 0 ||
         strcmp(type, "f32") == 0 ||
         strcmp(type, "f64") == 0 ||
         strcmp(type, "bool") == 0 ||
         strcmp(type, "char") == 0 ||
         strcmp(type, "string") == 0 ||
         strcmp(type, "void") == 0;
}
