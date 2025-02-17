#ifndef ZIR_TO_MIR_H
#define ZIR_TO_MIR_H

#include "mir.h"
#include "zir.h"
#include <stdbool.h>

// Translation context to maintain state during translation
typedef struct
{
    MIRModule *mir_module;     // Target MIR module
    MIRFunction *current_func; // Current function being processed
    MIRBlock *current_block;   // Current block being filled
    int next_reg;              // Next available register number

    // Memory management
    int next_stack_offset; // Next available stack offset for locals
} TranslationContext;

// Context management
TranslationContext *create_translation_context(void);
void free_translation_context(TranslationContext *ctx);

// Core translation functions
MIRModule *translate_zir_to_mir(ZIRModule *zir_module);
void translate_function(TranslationContext *ctx, ZIRFunction *func);
void translate_block(TranslationContext *ctx, ZIRBlock *block);
MIROperand translate_value(TranslationContext *ctx, ZIRValue *value);

// Value translation helpers
MIROperand translate_constant(TranslationContext *ctx, ZIRValue *value);
MIROperand translate_local(TranslationContext *ctx, ZIRValue *value);
MIROperand translate_binary(TranslationContext *ctx, ZIRValue *value);
MIROperand translate_call(TranslationContext *ctx, ZIRValue *value);

// Memory operations
MIROperand translate_load(TranslationContext *ctx, ZIRValue *value);
void translate_store(TranslationContext *ctx, ZIRValue *value);
MIROperand translate_alloca(TranslationContext *ctx, ZIRValue *value);

// Control flow translation
void translate_branch(TranslationContext *ctx, ZIRValue *value);
void translate_jump(TranslationContext *ctx, ZIRValue *value);
void translate_return(TranslationContext *ctx, ZIRValue *value);

// Utility functions
int get_next_reg(TranslationContext *ctx);
int allocate_stack_slot(TranslationContext *ctx, Type *type);
MIROpcode convert_binary_op(const char *zir_op);
char *gen_unique_label(TranslationContext *ctx, const char *prefix);

#endif // ZIR_TO_MIR_H