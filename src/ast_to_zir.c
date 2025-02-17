#include "include/ast_to_zir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Error reporting
void zir_gen_error(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    fprintf(stderr, "ZIR Generation Error: ");
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

// Helper functions for name generation
char *gen_temp_name(ZIRGenContext *ctx)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "tmp%d", ctx->temp_counter++);
    return strdup(buffer);
}

char *gen_block_label(ZIRGenContext *ctx)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "block%d", ctx->block_counter++);
    return strdup(buffer);
}

// Context management
ZIRGenContext *create_zir_gen_context(void)
{
    ZIRGenContext *ctx = malloc(sizeof(ZIRGenContext));
    ctx->module = create_zir_module();
    ctx->current_func = NULL;
    ctx->current_block = NULL;
    ctx->symbols = create_symbol_table(NULL);
    ctx->temp_counter = 0;
    ctx->block_counter = 0;
    return ctx;
}

void free_zir_gen_context(ZIRGenContext *ctx)
{
    // Note: Don't free module here as it's the return value
    destroy_symbol_table(ctx->symbols);
    free(ctx);
}

// Forward declarations for mutually recursive functions
ZIRValue *translate_expr(ZIRGenContext *ctx, ASTNode *node);
void translate_stmt(ZIRGenContext *ctx, ASTNode *node);

// Translation functions
ZIRValue *translate_literal(ZIRGenContext *ctx, ASTNode *node)
{
    Type *type = get_literal_type(node->data.literal.value);
    return create_zir_const(type, node->data.literal.value);
}

ZIRValue *translate_identifier(ZIRGenContext *ctx, ASTNode *node)
{
    Symbol *sym = lookup_symbol(ctx->symbols, node->data.identifier.name);
    if (!sym)
    {
        zir_gen_error("Undefined variable '%s'", node->data.identifier.name);
    }

    // Create a load instruction if this is a variable reference
    ZIRValue *var = create_zir_local(type_from_string(sym->type),
                                     node->data.identifier.name, true);
    return create_zir_load(var);
}

ZIRValue *translate_binary_expr(ZIRGenContext *ctx, ASTNode *node)
{
    ZIRValue *left = translate_expr(ctx, node->data.binary_expr.left);
    ZIRValue *right = translate_expr(ctx, node->data.binary_expr.right);
    return create_zir_binary(node->data.binary_expr.op, left, right);
}

ZIRValue *translate_unary_expr(ZIRGenContext *ctx, ASTNode *node)
{
    ZIRValue *operand = translate_expr(ctx, node->data.unary_expr.operand);
    return create_zir_unary(node->data.unary_expr.op, operand);
}

ZIRValue *translate_func_call(ZIRGenContext *ctx, ASTNode *node)
{
    // Translate all arguments
    ZIRValue **args = malloc(node->data.func_call.arg_count * sizeof(ZIRValue *));
    for (int i = 0; i < node->data.func_call.arg_count; i++)
    {
        args[i] = translate_expr(ctx, node->data.func_call.arguments[i]);
    }

    ZIRValue *call = create_zir_call(node->data.func_call.name, args,
                                     node->data.func_call.arg_count);
    free(args);
    return call;
}

// Translate any expression
ZIRValue *translate_expr(ZIRGenContext *ctx, ASTNode *node)
{
    switch (node->type)
    {
    case AST_LITERAL:
        return translate_literal(ctx, node);
    case AST_IDENTIFIER:
        return translate_identifier(ctx, node);
    case AST_BINARY_EXPR:
        return translate_binary_expr(ctx, node);
    case AST_UNARY_EXPR:
        return translate_unary_expr(ctx, node);
    case AST_FUNC_CALL:
        return translate_func_call(ctx, node);
    default:
        zir_gen_error("Unexpected expression node type: %d", node->type);
        return NULL;
    }
}

// Translate a variable declaration
void translate_var_decl(ZIRGenContext *ctx, ASTNode *node)
{
    Type *type = type_from_string(node->data.var_decl.type_annotation);

    // Create the local variable
    ZIRValue *var = create_zir_local(type, node->data.var_decl.identifier,
                                     !node->data.var_decl.is_const);

    // Add to symbol table
    add_symbol(ctx->symbols, node->data.var_decl.identifier,
               node->data.var_decl.type_annotation);

    // Translate initializer if present
    if (node->data.var_decl.initializer)
    {
        ZIRValue *init = translate_expr(ctx, node->data.var_decl.initializer);
        ZIRValue *store = create_zir_store(var, init);
        zir_block_add_instr(ctx->current_block, store);
    }
}

// Translate an if statement
void translate_if_stmt(ZIRGenContext *ctx, ASTNode *node)
{
    ZIRValue *cond = translate_expr(ctx, node->data.if_stmt.condition);

    ZIRBlock *then_block = create_zir_block(gen_block_label(ctx));
    ZIRBlock *else_block = node->data.if_stmt.else_block ? create_zir_block(gen_block_label(ctx)) : NULL;
    ZIRBlock *merge_block = create_zir_block(gen_block_label(ctx));

    // Create branch instruction
    ZIRValue *branch = create_zir_branch(cond, then_block, else_block ? else_block : merge_block);
    zir_block_add_instr(ctx->current_block, branch);

    // Translate then block
    ctx->current_block = then_block;
    translate_stmt(ctx, node->data.if_stmt.if_block);
    zir_block_add_instr(ctx->current_block, create_zir_jump(merge_block));

    // Translate else block if present
    if (else_block)
    {
        ctx->current_block = else_block;
        translate_stmt(ctx, node->data.if_stmt.else_block);
        zir_block_add_instr(ctx->current_block, create_zir_jump(merge_block));
    }

    // Add blocks to function
    zir_function_add_block(ctx->current_func, then_block);
    if (else_block)
        zir_function_add_block(ctx->current_func, else_block);
    zir_function_add_block(ctx->current_func, merge_block);

    ctx->current_block = merge_block;
}

// Translate a while loop
void translate_while_stmt(ZIRGenContext *ctx, ASTNode *node)
{
    ZIRBlock *cond_block = create_zir_block(gen_block_label(ctx));
    ZIRBlock *body_block = create_zir_block(gen_block_label(ctx));
    ZIRBlock *merge_block = create_zir_block(gen_block_label(ctx));

    // Jump to condition block
    zir_block_add_instr(ctx->current_block, create_zir_jump(cond_block));

    // Generate condition block
    ctx->current_block = cond_block;
    ZIRValue *cond = translate_expr(ctx, node->data.while_stmt.condition);
    zir_block_add_instr(ctx->current_block, create_zir_branch(cond, body_block, merge_block));

    // Generate body block
    ctx->current_block = body_block;
    translate_stmt(ctx, node->data.while_stmt.block);
    zir_block_add_instr(ctx->current_block, create_zir_jump(cond_block));

    // Add blocks to function
    zir_function_add_block(ctx->current_func, cond_block);
    zir_function_add_block(ctx->current_func, body_block);
    zir_function_add_block(ctx->current_func, merge_block);

    ctx->current_block = merge_block;
}

// Translate a for loop
void translate_for_stmt(ZIRGenContext *ctx, ASTNode *node)
{
    // Create blocks
    ZIRBlock *init_block = create_zir_block(gen_block_label(ctx));
    ZIRBlock *cond_block = create_zir_block(gen_block_label(ctx));
    ZIRBlock *body_block = create_zir_block(gen_block_label(ctx));
    ZIRBlock *merge_block = create_zir_block(gen_block_label(ctx));

    // Translate initializer
    ctx->current_block = init_block;
    Type *iter_type = type_from_string("i32"); // For range-based for loops
    ZIRValue *iter = create_zir_local(iter_type, node->data.for_stmt.iterator, true);
    ZIRValue *start = translate_expr(ctx, node->data.for_stmt.start_expr);
    zir_block_add_instr(ctx->current_block, create_zir_store(iter, start));
    zir_block_add_instr(ctx->current_block, create_zir_jump(cond_block));

    // Translate condition
    ctx->current_block = cond_block;
    ZIRValue *end = translate_expr(ctx, node->data.for_stmt.end_expr);
    ZIRValue *iter_val = create_zir_load(iter);
    ZIRValue *cond = create_zir_binary("<", iter_val, end);
    zir_block_add_instr(ctx->current_block, create_zir_branch(cond, body_block, merge_block));

    // Translate body
    ctx->current_block = body_block;
    translate_stmt(ctx, node->data.for_stmt.block);

    // Increment iterator
    ZIRValue *one = create_zir_const(iter_type, "1");
    ZIRValue *next = create_zir_binary("+", create_zir_load(iter), one);
    zir_block_add_instr(ctx->current_block, create_zir_store(iter, next));
    zir_block_add_instr(ctx->current_block, create_zir_jump(cond_block));

    // Add blocks to function
    zir_function_add_block(ctx->current_func, init_block);
    zir_function_add_block(ctx->current_func, cond_block);
    zir_function_add_block(ctx->current_func, body_block);
    zir_function_add_block(ctx->current_func, merge_block);

    ctx->current_block = merge_block;
}

// Translate a block of statements
void translate_block(ZIRGenContext *ctx, ASTNode *node)
{
    // Create new scope
    SymbolTable *old_symbols = ctx->symbols;
    ctx->symbols = create_symbol_table(old_symbols);

    // Translate each statement
    for (int i = 0; i < node->data.block.stmt_count; i++)
    {
        translate_stmt(ctx, node->data.block.statements[i]);
    }

    // Restore old scope
    destroy_symbol_table(ctx->symbols);
    ctx->symbols = old_symbols;
}

// Translate a function definition
void translate_func_def(ZIRGenContext *ctx, ASTNode *node)
{
    Type *return_type = type_from_string(node->data.func_def.return_type ? node->data.func_def.return_type : "void");

    // Create function
    ZIRFunction *func = create_zir_function(node->data.func_def.name, return_type, false);
    ctx->current_func = func;

    // Create entry block
    ZIRBlock *entry = create_zir_block("entry");
    ctx->current_block = entry;
    zir_function_add_block(func, entry);

    // Add parameters
    for (int i = 0; i < node->data.func_def.param_count; i++)
    {
        ASTNode *param = node->data.func_def.parameters[i];
        Type *param_type = type_from_string(param->data.var_decl.type_annotation);
        zir_function_add_param(func, param->data.var_decl.identifier, param_type);
        add_symbol(ctx->symbols, param->data.var_decl.identifier,
                   param->data.var_decl.type_annotation);
    }

    // Translate function body
    translate_block(ctx, node->data.func_def.body);

    // Add function to module
    zir_module_add_function(ctx->module, func);
    ctx->current_func = NULL;
    ctx->current_block = NULL;
}

// Translate a return statement
void translate_return_stmt(ZIRGenContext *ctx, ASTNode *node)
{
    if (!ctx->current_func)
    {
        zir_gen_error("Return statement outside of function");
    }

    // For void return type
    if (!node->data.return_stmt.expr)
    {
        if (ctx->current_func->return_type->kind != TYPE_VOID)
        {
            zir_gen_error("Expected return value in non-void function");
        }
        zir_block_add_instr(ctx->current_block, create_zir_return(NULL));
        return;
    }

    // For non-void return type
    ZIRValue *return_value = translate_expr(ctx, node->data.return_stmt.expr);
    if (!return_value)
    {
        zir_gen_error("Failed to translate return expression");
    }

    // Type check the return value
    if (!type_is_safe_for(return_value->type, ctx->current_func->return_type))
    {
        zir_gen_error("Return value type does not match function return type");
    }

    // Create return instruction
    ZIRValue *return_instr = create_zir_return(return_value);
    zir_block_add_instr(ctx->current_block, return_instr);
}

// Translate any statement
void translate_stmt(ZIRGenContext *ctx, ASTNode *node)
{
    switch (node->type)
    {
    case AST_VAR_DECL:
        translate_var_decl(ctx, node);
        break;
    case AST_IF_STMT:
        translate_if_stmt(ctx, node);
        break;
    case AST_WHILE_STMT:
        translate_while_stmt(ctx, node);
        break;
    case AST_FOR_STMT:
        translate_for_stmt(ctx, node);
        break;
    case AST_BLOCK:
        translate_block(ctx, node);
        break;
    case AST_FUNC_DEF:
        translate_func_def(ctx, node);
        break;
    case AST_EXPR_STMT:
    {
        ZIRValue *val = translate_expr(ctx, node->data.expr_stmt.expr);
        zir_block_add_instr(ctx->current_block, val);
        break;
    }
    case AST_RETURN_STMT:
        translate_return_stmt(ctx, node);
        break;
    default:
        zir_gen_error("Unexpected statement node type: %d", node->type);
    }
}

// Main entry point
ZIRModule *translate_ast_to_zir(ASTNode *ast)
{
    if (!ast)
        return NULL;

    ZIRGenContext *ctx = create_zir_gen_context();

    // Create main function for top-level code
    Type *void_type = type_from_string("void");
    ZIRFunction *main_func = create_zir_function("main", void_type, false);
    ctx->current_func = main_func;

    // Create entry block
    ZIRBlock *entry = create_zir_block("entry");
    ctx->current_block = entry;
    zir_function_add_block(main_func, entry);

    // Translate the AST
    translate_stmt(ctx, ast);

    // Add main function to module
    zir_module_add_function(ctx->module, main_func);

    // Get the module and clean up
    ZIRModule *module = ctx->module;
    free_zir_gen_context(ctx);
    return module;
}