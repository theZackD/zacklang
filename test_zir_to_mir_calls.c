#include "include/zir_to_mir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Test simple function call with no arguments
void test_void_call(void)
{
    printf("\nTesting void function call...\n");

    TranslationContext *ctx = create_translation_context();

    // Create function and block context
    Type *void_type = create_type(TYPE_VOID);
    MIRFunction *func = create_mir_function("test", void_type);
    MIRBlock *entry = create_mir_block("entry");
    ctx->current_func = func;
    ctx->current_block = entry;

    // Create a call to print() with no arguments
    ZIRValue *call = create_zir_call("print", NULL, 0);
    translate_value(ctx, call);

    // Verify call instruction
    assert(entry->first_instr != NULL);
    assert(entry->first_instr->op == MIR_CALL);
    assert(entry->first_instr->operand_count == 1); // Just the function name
    assert(entry->first_instr->operands[0].kind == MIR_LABEL);
    assert(strcmp(entry->first_instr->operands[0].value.label, "print") == 0);

    // Clean up
    free_zir_value(call);
    free_type(void_type);
    free_mir_function(func);
    free_translation_context(ctx);
    printf("✓ Void function call test passed\n");
}

// Test function call with arguments
void test_call_with_args(void)
{
    printf("\nTesting function call with arguments...\n");

    TranslationContext *ctx = create_translation_context();

    // Create function and block context
    Type *i32_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("test", i32_type);
    MIRBlock *entry = create_mir_block("entry");
    ctx->current_func = func;
    ctx->current_block = entry;

    // Create arguments
    ZIRValue *arg1 = create_zir_const(i32_type, "42");
    ZIRValue *arg2 = create_zir_const(i32_type, "10");
    ZIRValue *args[] = {arg1, arg2};

    // Create a call to add(42, 10)
    ZIRValue *call = create_zir_call("add", args, 2);
    call->type = i32_type; // Set return type
    MIROperand result = translate_value(ctx, call);

    // Verify call instruction
    assert(entry->first_instr != NULL);
    assert(entry->first_instr->op == MIR_CALL);
    assert(entry->first_instr->operand_count == 4); // result, function, arg1, arg2

    // Verify operands in order: result, function, args...
    assert(entry->first_instr->operands[0].kind == MIR_REG);   // Result register
    assert(entry->first_instr->operands[1].kind == MIR_LABEL); // Function name
    assert(strcmp(entry->first_instr->operands[1].value.label, "add") == 0);
    assert(entry->first_instr->operands[2].kind == MIR_IMM); // First argument
    assert(entry->first_instr->operands[2].value.imm == 42);
    assert(entry->first_instr->operands[3].kind == MIR_IMM); // Second argument
    assert(entry->first_instr->operands[3].value.imm == 10);

    // Verify result
    assert(result.kind == MIR_REG);
    assert(result.type == i32_type);

    // Clean up
    free_zir_value(call);
    free_zir_value(arg1);
    free_zir_value(arg2);
    free_type(i32_type);
    free_mir_function(func);
    free_translation_context(ctx);
    printf("✓ Function call with arguments test passed\n");
}

// Test nested function calls
void test_nested_calls(void)
{
    printf("\nTesting nested function calls...\n");

    TranslationContext *ctx = create_translation_context();

    // Create function and block context
    Type *i32_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("test", i32_type);
    MIRBlock *entry = create_mir_block("entry");
    ctx->current_func = func;
    ctx->current_block = entry;

    // Create inner call: add(40, 2)
    ZIRValue *inner_arg1 = create_zir_const(i32_type, "40");
    ZIRValue *inner_arg2 = create_zir_const(i32_type, "2");
    ZIRValue *inner_args[] = {inner_arg1, inner_arg2};
    ZIRValue *inner_call = create_zir_call("add", inner_args, 2);
    inner_call->type = i32_type; // Set return type

    // Create outer call: mul(add(40, 2), 10)
    ZIRValue *outer_arg2 = create_zir_const(i32_type, "10");
    ZIRValue *outer_args[] = {inner_call, outer_arg2};
    ZIRValue *outer_call = create_zir_call("mul", outer_args, 2);
    outer_call->type = i32_type; // Set return type

    MIROperand result = translate_value(ctx, outer_call);

    // Verify instruction sequence
    MIRInstr *instr = entry->first_instr;
    assert(instr != NULL);
    assert(instr->op == MIR_CALL); // Inner call to add
    assert(instr->operand_count == 4);
    assert(instr->operands[0].kind == MIR_REG);   // Result register
    assert(instr->operands[1].kind == MIR_LABEL); // Function name
    assert(strcmp(instr->operands[1].value.label, "add") == 0);

    instr = instr->next;
    assert(instr != NULL);
    assert(instr->op == MIR_CALL); // Outer call to mul
    assert(instr->operand_count == 4);
    assert(instr->operands[0].kind == MIR_REG);   // Result register
    assert(instr->operands[1].kind == MIR_LABEL); // Function name
    assert(strcmp(instr->operands[1].value.label, "mul") == 0);
    assert(instr->operands[2].kind == MIR_REG); // Result of inner call
    assert(instr->operands[3].kind == MIR_IMM); // Constant 10

    // Verify final result
    assert(result.kind == MIR_REG);
    assert(result.type == i32_type);

    // Clean up
    free_zir_value(outer_call);
    free_zir_value(inner_call);
    free_zir_value(inner_arg1);
    free_zir_value(inner_arg2);
    free_zir_value(outer_arg2);
    free_type(i32_type);
    free_mir_function(func);
    free_translation_context(ctx);
    printf("✓ Nested function calls test passed\n");
}

int main(void)
{
    printf("Running ZIR to MIR function call tests...\n");

    test_void_call();
    test_call_with_args();
    test_nested_calls();

    printf("\nAll ZIR to MIR function call tests passed! ✓\n");
    return 0;
}