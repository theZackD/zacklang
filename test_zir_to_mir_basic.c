#include "include/zir_to_mir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Helper function for string comparison
static void assert_str_eq(const char *actual, const char *expected)
{
    if (strcmp(actual, expected) != 0)
    {
        printf("Expected '%s' but got '%s'\n", expected, actual);
        assert(0);
    }
}

// Test context creation and management
void test_context_management(void)
{
    printf("\nTesting context management...\n");

    TranslationContext *ctx = create_translation_context();
    assert(ctx != NULL);
    assert(ctx->mir_module != NULL);
    assert(ctx->current_func == NULL);
    assert(ctx->current_block == NULL);
    assert(ctx->next_reg == 0);

    free_translation_context(ctx);
    printf("✓ Context management test passed\n");
}

// Test constant translation
void test_constant_translation(void)
{
    printf("\nTesting constant translation...\n");

    TranslationContext *ctx = create_translation_context();

    // Create a constant ZIR value
    Type *i32_type = create_type(TYPE_I32);
    ZIRValue *const_val = create_zir_const(i32_type, "42");

    // Translate to MIR
    MIROperand result = translate_constant(ctx, const_val);

    // Verify result
    assert(result.kind == MIR_IMM);
    assert(result.type == i32_type);
    assert(result.value.imm == 42);

    free_zir_value(const_val);
    free_type(i32_type);
    free_translation_context(ctx);
    printf("✓ Constant translation test passed\n");
}

// Test local variable translation
void test_local_translation(void)
{
    printf("\nTesting local variable translation...\n");

    TranslationContext *ctx = create_translation_context();

    // Create a local ZIR value
    Type *i32_type = create_type(TYPE_I32);
    ZIRValue *local_val = create_zir_local(i32_type, "x", true);

    // Translate to MIR
    MIROperand result = translate_local(ctx, local_val);

    // Verify result
    assert(result.kind == MIR_REG);
    assert(result.type == i32_type);
    assert(result.value.reg_num == 0); // Should be first register

    // Create another local and verify register number increases
    ZIRValue *local_val2 = create_zir_local(i32_type, "y", true);
    MIROperand result2 = translate_local(ctx, local_val2);
    assert(result2.value.reg_num == 1);

    free_zir_value(local_val);
    free_zir_value(local_val2);
    free_type(i32_type);
    free_translation_context(ctx);
    printf("✓ Local variable translation test passed\n");
}

// Test binary operation translation
void test_binary_translation(void)
{
    printf("\nTesting binary operation translation...\n");

    TranslationContext *ctx = create_translation_context();

    // Create function and block context
    Type *i32_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("test", i32_type);
    MIRBlock *block = create_mir_block("entry");
    ctx->current_func = func;
    ctx->current_block = block;

    // Create operands
    ZIRValue *left = create_zir_const(i32_type, "40");
    ZIRValue *right = create_zir_const(i32_type, "2");

    printf("Debug: Left operand type kind: %d\n", left->type->kind);
    printf("Debug: Right operand type kind: %d\n", right->type->kind);

    ZIRValue *add = create_zir_binary("+", left, right);
    printf("Debug: Binary operation type kind: %d\n", add->type->kind);

    // Translate binary operation
    MIROperand result = translate_binary(ctx, add);
    printf("Debug: Result type kind: %d\n", result.type->kind);
    printf("Debug: Expected type kind: %d\n", i32_type->kind);

    // Verify result
    assert(result.kind == MIR_REG);
    assert(result.type->kind == i32_type->kind); // Compare kinds instead of pointers

    // Verify instruction was created
    assert(block->first_instr != NULL);
    assert(block->first_instr->op == MIR_ADD);
    assert(block->first_instr->operand_count == 3);

    free_zir_value(add);
    free_zir_value(left);
    free_zir_value(right);
    free_type(i32_type);
    free_mir_function(func);
    free_translation_context(ctx);
    printf("✓ Binary operation translation test passed\n");
}

int main(void)
{
    printf("Running ZIR to MIR basic translation tests...\n");

    test_context_management();
    test_constant_translation();
    test_local_translation();
    test_binary_translation();

    printf("\nAll ZIR to MIR basic translation tests passed! ✓\n");
    return 0;
}