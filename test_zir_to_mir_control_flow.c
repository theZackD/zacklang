#include "include/zir_to_mir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Helper function to create a simple if statement in ZIR
static ZIRValue *create_test_if_statement(Type *condition_type)
{
    // Create blocks
    ZIRBlock *then_block = create_zir_block("then");
    ZIRBlock *else_block = create_zir_block("else");

    // Create condition (x > 0)
    ZIRValue *x = create_zir_local(condition_type, "x", false);
    ZIRValue *zero = create_zir_const(condition_type, "0");
    ZIRValue *condition = create_zir_binary(">", x, zero);

    // Create branch instruction using create_zir_branch
    ZIRValue *branch = create_zir_branch(condition, then_block, else_block);

    return branch;
}

// Test branch translation
void test_branch_translation(void)
{
    printf("\nTesting branch translation...\n");

    TranslationContext *ctx = create_translation_context();

    // Create function and entry block context
    Type *i32_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("test", i32_type);
    MIRBlock *entry = create_mir_block("entry");
    ctx->current_func = func;
    ctx->current_block = entry;

    // Create and translate if statement
    ZIRValue *branch = create_test_if_statement(i32_type);
    translate_branch(ctx, branch);

    // Verify that branch instruction was created
    assert(entry->first_instr != NULL);
    assert(entry->first_instr->op == MIR_CMP_GT); // Compare instruction
    assert(entry->first_instr->next != NULL);
    assert(entry->first_instr->next->op == MIR_BRANCH); // Branch instruction

    // Verify branch targets
    assert(entry->first_instr->next->operands[1].kind == MIR_LABEL); // then target
    assert(entry->first_instr->next->operands[2].kind == MIR_LABEL); // else target

    // Clean up
    free_zir_value(branch);
    free_type(i32_type);
    free_mir_function(func);
    free_translation_context(ctx);
    printf("✓ Branch translation test passed\n");
}

// Test jump translation
void test_jump_translation(void)
{
    printf("\nTesting jump translation...\n");

    TranslationContext *ctx = create_translation_context();

    // Create function and blocks
    Type *i32_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("test", i32_type);
    MIRBlock *entry = create_mir_block("entry");
    ZIRBlock *target = create_zir_block("target");

    ctx->current_func = func;
    ctx->current_block = entry;

    // Create and translate jump
    ZIRValue *jump = create_zir_jump(target);
    translate_jump(ctx, jump);

    // Verify jump instruction
    assert(entry->first_instr != NULL);
    assert(entry->first_instr->op == MIR_JUMP);
    assert(entry->first_instr->operand_count == 1);
    assert(entry->first_instr->operands[0].kind == MIR_LABEL);
    assert(strcmp(entry->first_instr->operands[0].value.label, "target") == 0);

    // Clean up
    free_zir_value(jump);
    free_zir_block(target);
    free_type(i32_type);
    free_mir_function(func);
    free_translation_context(ctx);
    printf("✓ Jump translation test passed\n");
}

// Test return translation
void test_return_translation(void)
{
    printf("\nTesting return translation...\n");

    TranslationContext *ctx = create_translation_context();

    // Create function and block
    Type *i32_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("test", i32_type);
    MIRBlock *entry = create_mir_block("entry");
    ctx->current_func = func;
    ctx->current_block = entry;

    // Test void return
    ZIRValue *void_return = create_zir_return(NULL);
    translate_return(ctx, void_return);

    // Verify void return instruction
    assert(entry->first_instr != NULL);
    assert(entry->first_instr->op == MIR_RET);
    assert(entry->first_instr->operand_count == 0);

    // Create new block for value return test
    MIRBlock *block2 = create_mir_block("block2");
    ctx->current_block = block2;

    // Test value return
    ZIRValue *const_val = create_zir_const(i32_type, "42");
    ZIRValue *value_return = create_zir_return(const_val);
    translate_return(ctx, value_return);

    // Verify value return instruction
    assert(block2->first_instr != NULL);
    assert(block2->first_instr->op == MIR_RET);
    assert(block2->first_instr->operand_count == 1);
    assert(block2->first_instr->operands[0].kind == MIR_IMM);
    assert(block2->first_instr->operands[0].value.imm == 42);

    // Clean up
    free_zir_value(void_return);
    free_zir_value(value_return);
    free_zir_value(const_val);
    free_type(i32_type);
    free_mir_function(func);
    free_translation_context(ctx);
    printf("✓ Return translation test passed\n");
}

int main(void)
{
    printf("Running ZIR to MIR control flow translation tests...\n");

    test_branch_translation();
    test_jump_translation();
    test_return_translation();

    printf("\nAll ZIR to MIR control flow translation tests passed! ✓\n");
    return 0;
}