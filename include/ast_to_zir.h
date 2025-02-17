#ifndef AST_TO_ZIR_H
#define AST_TO_ZIR_H

#include "ast.h"
#include "zir.h"
#include "symbol_table.h"

// Context for ZIR generation
typedef struct
{
    ZIRModule *module;         // Current module being built
    ZIRFunction *current_func; // Current function being processed
    ZIRBlock *current_block;   // Current block being filled
    SymbolTable *symbols;      // Symbol table for variable lookup
    int temp_counter;          // Counter for generating unique temporary names
    int block_counter;         // Counter for generating unique block labels
} ZIRGenContext;

// Create a new ZIR generation context
ZIRGenContext *create_zir_gen_context(void);

// Free a ZIR generation context
void free_zir_gen_context(ZIRGenContext *ctx);

// Main entry point: translate AST to ZIR
ZIRModule *translate_ast_to_zir(ASTNode *ast);

// Helper functions for name generation
char *gen_temp_name(ZIRGenContext *ctx);
char *gen_block_label(ZIRGenContext *ctx);

// Translation functions for different AST node types
ZIRValue *translate_expr(ZIRGenContext *ctx, ASTNode *node);
void translate_stmt(ZIRGenContext *ctx, ASTNode *node);
void translate_var_decl(ZIRGenContext *ctx, ASTNode *node);
void translate_func_def(ZIRGenContext *ctx, ASTNode *node);
ZIRValue *translate_binary_expr(ZIRGenContext *ctx, ASTNode *node);
ZIRValue *translate_unary_expr(ZIRGenContext *ctx, ASTNode *node);
ZIRValue *translate_literal(ZIRGenContext *ctx, ASTNode *node);
ZIRValue *translate_identifier(ZIRGenContext *ctx, ASTNode *node);
ZIRValue *translate_func_call(ZIRGenContext *ctx, ASTNode *node);
void translate_if_stmt(ZIRGenContext *ctx, ASTNode *node);
void translate_while_stmt(ZIRGenContext *ctx, ASTNode *node);
void translate_for_stmt(ZIRGenContext *ctx, ASTNode *node);
void translate_block(ZIRGenContext *ctx, ASTNode *node);
void translate_return_stmt(ZIRGenContext *ctx, ASTNode *node);

// Error reporting
void zir_gen_error(const char *message, ...);

#endif // AST_TO_ZIR_H