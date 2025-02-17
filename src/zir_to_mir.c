#include "include/mir.h"
#include "include/zir.h"
#include "include/zir_to_mir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function for memory allocation
static void *safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    return ptr;
}

// Helper function for string duplication
static char *safe_strdup(const char *str)
{
    char *dup = strdup(str);
    if (!dup)
    {
        fprintf(stderr, "String duplication failed\n");
        exit(1);
    }
    return dup;
}

// Create translation context
TranslationContext *create_translation_context(void)
{
    TranslationContext *ctx = safe_malloc(sizeof(TranslationContext));
    ctx->mir_module = create_mir_module();
    ctx->current_func = NULL;
    ctx->current_block = NULL;
    ctx->next_reg = 0;
    ctx->next_stack_offset = 0; // Initialize stack offset
    return ctx;
}

// Free translation context
void free_translation_context(TranslationContext *ctx)
{
    // Note: Don't free mir_module here as it's the return value
    free(ctx);
}

// Get next available register
int get_next_reg(TranslationContext *ctx)
{
    return ctx->next_reg++;
}

// Allocate stack slot for a variable
int allocate_stack_slot(TranslationContext *ctx, Type *type)
{
    // For now, treat all types as 8-byte aligned
    const int ALIGNMENT = 8;

    // Align the offset
    ctx->next_stack_offset = (ctx->next_stack_offset + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

    // Get current offset and increment for next allocation
    int offset = ctx->next_stack_offset;
    ctx->next_stack_offset += ALIGNMENT;

    return offset;
}

// Generate a unique label
char *gen_unique_label(TranslationContext *ctx, const char *prefix)
{
    static int counter = 0;
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s_%d", prefix, counter++);
    return safe_strdup(buffer);
}

// Convert ZIR binary operator to MIR opcode
MIROpcode convert_binary_op(const char *zir_op)
{
    if (strcmp(zir_op, "+") == 0)
        return MIR_ADD;
    if (strcmp(zir_op, "-") == 0)
        return MIR_SUB;
    if (strcmp(zir_op, "*") == 0)
        return MIR_MUL;
    if (strcmp(zir_op, "/") == 0)
        return MIR_DIV;
    if (strcmp(zir_op, "%") == 0)
        return MIR_MOD;
    if (strcmp(zir_op, "==") == 0)
        return MIR_CMP_EQ;
    if (strcmp(zir_op, "!=") == 0)
        return MIR_CMP_NE;
    if (strcmp(zir_op, "<") == 0)
        return MIR_CMP_LT;
    if (strcmp(zir_op, "<=") == 0)
        return MIR_CMP_LE;
    if (strcmp(zir_op, ">") == 0)
        return MIR_CMP_GT;
    if (strcmp(zir_op, ">=") == 0)
        return MIR_CMP_GE;

    fprintf(stderr, "Unknown binary operator: %s\n", zir_op);
    exit(1);
}

// Forward declarations
MIROperand translate_value(TranslationContext *ctx, ZIRValue *value);

// Memory operations
MIROperand translate_alloca(TranslationContext *ctx, ZIRValue *value)
{
    // Allocate stack slot
    int offset = allocate_stack_slot(ctx, value->data.alloca.alloc_type);

    // Create memory operand relative to frame pointer
    return create_mem_operand(FRAME_POINTER_REG, -offset, value->data.alloca.alloc_type);
}

MIROperand translate_load(TranslationContext *ctx, ZIRValue *value)
{
    // Get address operand
    MIROperand addr = translate_value(ctx, value->data.load.address);

    // Create destination register
    int result_reg = get_next_reg(ctx);
    MIROperand result = create_reg_operand(result_reg, value->type);

    // Create load instruction
    MIROperand operands[] = {result, addr};
    mir_block_add_instr(ctx->current_block,
                        create_mir_instr(MIR_LOAD, operands, 2));

    return result;
}

// Translate a store instruction
void translate_store(TranslationContext *ctx, ZIRValue *value)
{
    // Get address operand
    MIROperand addr = translate_value(ctx, value->data.store.address);

    // Get value operand
    MIROperand val = translate_value(ctx, value->data.store.value);

    // Create store instruction
    MIROperand operands[] = {addr, val};
    MIRInstr *instr = create_mir_instr(MIR_STORE, operands, 2);
    mir_block_add_instr(ctx->current_block, instr);
}

// Translate a constant value
MIROperand translate_constant(TranslationContext *ctx, ZIRValue *value)
{
    // For now, we only handle integer constants
    if (value->type->kind != TYPE_I32 && value->type->kind != TYPE_I64)
    {
        fprintf(stderr, "Only integer constants supported for now\n");
        exit(1);
    }

    int64_t const_val = atoll(value->data.constant.literal_value);
    return create_imm_operand(const_val, value->type);
}

// Translate a local variable
MIROperand translate_local(TranslationContext *ctx, ZIRValue *value)
{
    // For now, just allocate a new register for each local
    int reg = get_next_reg(ctx);
    return create_reg_operand(reg, value->type);
}

// Translate a binary operation
MIROperand translate_binary(TranslationContext *ctx, ZIRValue *value)
{
    // First translate the operands
    MIROperand left = translate_value(ctx, value->data.binary.left);
    MIROperand right = translate_value(ctx, value->data.binary.right);

    // Create result register
    int result_reg = get_next_reg(ctx);
    MIROperand result = create_reg_operand(result_reg, value->type);

    // Create instruction
    MIROpcode op = convert_binary_op(value->data.binary.op);
    MIROperand operands[] = {result, left, right};

    // Add the instruction to the current block
    MIRInstr *instr = create_mir_instr(op, operands, 3);
    mir_block_add_instr(ctx->current_block, instr);

    return result;
}

// Translate a branch instruction
void translate_branch(TranslationContext *ctx, ZIRValue *value)
{
    // First translate the condition
    MIROperand cond = translate_value(ctx, value->data.branch.condition);

    // Create label operands for the branch targets
    MIROperand then_label = create_label_operand(value->data.branch.then_block->label);
    MIROperand else_label = create_label_operand(
        value->data.branch.else_block ? value->data.branch.else_block->label
                                      : value->data.branch.then_block->label);

    // Create branch instruction
    MIROperand operands[] = {cond, then_label, else_label};
    mir_block_add_instr(ctx->current_block,
                        create_mir_instr(MIR_BRANCH, operands, 3));
}

// Translate a jump instruction
void translate_jump(TranslationContext *ctx, ZIRValue *value)
{
    MIROperand target = create_label_operand(value->data.jump.target->label);
    mir_block_add_instr(ctx->current_block,
                        create_mir_instr(MIR_JUMP, &target, 1));
}

// Translate a return instruction
void translate_return(TranslationContext *ctx, ZIRValue *value)
{
    if (value->data.ret.value)
    {
        MIROperand ret_val = translate_value(ctx, value->data.ret.value);
        mir_block_add_instr(ctx->current_block,
                            create_mir_instr(MIR_RET, &ret_val, 1));
    }
    else
    {
        mir_block_add_instr(ctx->current_block,
                            create_mir_instr(MIR_RET, NULL, 0));
    }
}

// Translate a function call
MIROperand translate_call(TranslationContext *ctx, ZIRValue *value)
{
    // First translate all arguments
    MIROperand *arg_operands = NULL;
    int arg_count = value->data.call.arg_count;

    if (arg_count > 0)
    {
        arg_operands = safe_malloc(arg_count * sizeof(MIROperand));
        for (int i = 0; i < arg_count; i++)
        {
            arg_operands[i] = translate_value(ctx, value->data.call.args[i]);
        }
    }

    // Create function name operand
    MIROperand func_name = create_label_operand(value->data.call.func_name);

    // Create result register if function returns a value
    MIROperand result;
    int total_operands;

    if (value->type != NULL && value->type->kind != TYPE_VOID)
    {
        // Function returns a value
        int result_reg = get_next_reg(ctx);
        result = create_reg_operand(result_reg, value->type);
        total_operands = arg_count + 2; // result + function + args
    }
    else
    {
        // Void function
        result = (MIROperand){0};
        total_operands = arg_count + 1; // function + args
    }

    // Create operands array
    MIROperand *operands = safe_malloc(total_operands * sizeof(MIROperand));
    int op_idx = 0;

    // For non-void functions, first operand is the result register
    if (value->type != NULL && value->type->kind != TYPE_VOID)
    {
        operands[op_idx++] = result;
    }

    // Next operand is always the function name
    operands[op_idx++] = func_name;

    // Add argument operands
    for (int i = 0; i < arg_count; i++)
    {
        operands[op_idx++] = arg_operands[i];
    }

    // Create and add call instruction
    MIRInstr *call_instr = create_mir_instr(MIR_CALL, operands, total_operands);
    mir_block_add_instr(ctx->current_block, call_instr);

    // Clean up
    if (arg_operands)
    {
        free(arg_operands);
    }
    free(operands);

    // Return result register for non-void functions
    if (value->type != NULL && value->type->kind != TYPE_VOID)
    {
        return result;
    }
    else
    {
        return (MIROperand){0};
    }
}

// Main value translation function
MIROperand translate_value(TranslationContext *ctx, ZIRValue *value)
{
    switch (value->kind)
    {
    case ZIR_CONST:
        return translate_constant(ctx, value);
    case ZIR_LOCAL:
        return translate_local(ctx, value);
    case ZIR_BINARY:
        return translate_binary(ctx, value);
    case ZIR_LOAD:
        return translate_load(ctx, value);
    case ZIR_ALLOCA:
        return translate_alloca(ctx, value);
    case ZIR_CALL:
        return translate_call(ctx, value);
    case ZIR_BRANCH:
        translate_branch(ctx, value);
        return (MIROperand){0}; // Branches don't produce values
    case ZIR_JUMP:
        translate_jump(ctx, value);
        return (MIROperand){0}; // Jumps don't produce values
    case ZIR_RETURN:
        translate_return(ctx, value);
        return (MIROperand){0}; // Returns don't produce values
    case ZIR_STORE:
        translate_store(ctx, value);
        return (MIROperand){0}; // Stores don't produce values
    default:
        fprintf(stderr, "Unsupported ZIR value kind: %d\n", value->kind);
        exit(1);
    }
}

// Translate a block
void translate_block(TranslationContext *ctx, ZIRBlock *block)
{
    // Create new MIR block
    MIRBlock *mir_block = create_mir_block(block->label);
    ctx->current_block = mir_block;

    // Translate each instruction
    for (int i = 0; i < block->instr_count; i++)
    {
        translate_value(ctx, block->instructions[i]);
    }

    // Add block to current function
    mir_function_add_block(ctx->current_func, mir_block);
}

// Translate a function
void translate_function(TranslationContext *ctx, ZIRFunction *func)
{
    // Create MIR function
    MIRFunction *mir_func = create_mir_function(func->name, func->return_type);
    ctx->current_func = mir_func;
    ctx->next_reg = 0;

    // Add parameters
    for (int i = 0; i < func->param_count; i++)
    {
        mir_function_add_param(mir_func, func->params[i].name,
                               func->params[i].type);
    }

    // Translate entry block
    translate_block(ctx, func->entry_block);

    // Set register count and add to module
    mir_func->reg_count = ctx->next_reg;
    mir_module_add_function(ctx->mir_module, mir_func);
}

// Main translation function
MIRModule *translate_zir_to_mir(ZIRModule *zir_module)
{
    TranslationContext *ctx = create_translation_context();

    // Translate each function
    for (int i = 0; i < zir_module->func_count; i++)
    {
        translate_function(ctx, zir_module->functions[i]);
    }

    // Get result and clean up
    MIRModule *result = ctx->mir_module;
    free_translation_context(ctx);
    return result;
}