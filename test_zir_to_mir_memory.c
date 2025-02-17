#include "include/zir_to_mir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Test allocation and store
void test_alloca_and_store(void)
{
    printf("\nTesting allocation and store...\n");

    TranslationContext *ctx = create_translation_context();

    // Create function and block context
    Type *i32_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("test", i32_type);
    MIRBlock *entry = create_mir_block("entry");
    ctx->current_func = func;
    ctx->current_block = entry;

    // Create a local variable allocation
    ZIRValue *alloca = create_zir_alloca(i32_type, true);
    MIROperand addr = translate_alloca(ctx, alloca);

    // Verify allocation
    assert(addr.kind == MIR_MEM);
    assert(addr.type == i32_type);

    // Create and store a constant
    ZIRValue *const_val = create_zir_const(i32_type, "42");
    ZIRValue *store = create_zir_store(alloca, const_val);
    translate_store(ctx, store);

    // Verify store instruction
    assert(entry->first_instr != NULL);
    assert(entry->first_instr->op == MIR_STORE);
    assert(entry->first_instr->operand_count == 2);
    assert(entry->first_instr->operands[0].kind == MIR_MEM); // destination
    assert(entry->first_instr->operands[1].kind == MIR_IMM); // value
    assert(entry->first_instr->operands[1].value.imm == 42);

    // Clean up
    free_zir_value(alloca);
    free_zir_value(const_val);
    free_zir_value(store);
    free_type(i32_type);
    free_mir_function(func);
    free_translation_context(ctx);
    printf("✓ Allocation and store test passed\n");
}

// Test load operation
void test_load(void)
{
    printf("\nTesting load operation...\n");

    TranslationContext *ctx = create_translation_context();

    // Create function and block context
    Type *i32_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("test", i32_type);
    MIRBlock *entry = create_mir_block("entry");
    ctx->current_func = func;
    ctx->current_block = entry;

    // Create a local variable and store a value
    ZIRValue *alloca = create_zir_alloca(i32_type, true);
    MIROperand addr = translate_alloca(ctx, alloca);

    ZIRValue *const_val = create_zir_const(i32_type, "42");
    ZIRValue *store = create_zir_store(alloca, const_val);
    translate_store(ctx, store);

    // Create and translate load
    ZIRValue *load = create_zir_load(alloca);
    MIROperand result = translate_load(ctx, load);

    // Verify load instruction
    MIRInstr *load_instr = entry->first_instr->next; // After store
    assert(load_instr != NULL);
    assert(load_instr->op == MIR_LOAD);
    assert(load_instr->operand_count == 2);
    assert(load_instr->operands[0].kind == MIR_REG); // destination register
    assert(load_instr->operands[1].kind == MIR_MEM); // source address

    // Verify load result
    assert(result.kind == MIR_REG);
    assert(result.type == i32_type);

    // Clean up
    free_zir_value(alloca);
    free_zir_value(const_val);
    free_zir_value(store);
    free_zir_value(load);
    free_type(i32_type);
    free_mir_function(func);
    free_translation_context(ctx);
    printf("✓ Load operation test passed\n");
}

// Test variable update
void test_variable_update(void)
{
    printf("\nTesting variable update...\n");

    TranslationContext *ctx = create_translation_context();

    // Create function and block context
    Type *i32_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("test", i32_type);
    MIRBlock *entry = create_mir_block("entry");
    ctx->current_func = func;
    ctx->current_block = entry;

    // Create a mutable local variable
    ZIRValue *alloca = create_zir_alloca(i32_type, true);
    MIROperand addr = translate_alloca(ctx, alloca);

    // Initial store
    ZIRValue *const_val1 = create_zir_const(i32_type, "42");
    ZIRValue *store1 = create_zir_store(alloca, const_val1);
    translate_store(ctx, store1);

    // Load the value
    ZIRValue *load = create_zir_load(alloca);
    MIROperand load_result = translate_load(ctx, load);

    // Create constant 1
    ZIRValue *const_one = create_zir_const(i32_type, "1");

    // Add 1 to the loaded value
    ZIRValue *add = create_zir_binary("+", load, const_one);
    MIROperand add_result = translate_binary(ctx, add);

    // Store back
    ZIRValue *store2 = create_zir_store(alloca, add);
    translate_store(ctx, store2);

    // Debug: Print instruction sequence
    printf("Debug: Instruction sequence:\n");
    MIRInstr *debug_instr = entry->first_instr;
    int instr_count = 0;
    while (debug_instr)
    {
        printf("Instruction %d: op = %d\n", instr_count++, debug_instr->op);
        debug_instr = debug_instr->next;
    }

    // Verify the sequence of instructions
    MIRInstr *instr = entry->first_instr;
    int found_store = 0;
    int found_load = 0;
    int found_add = 0;
    int found_final_store = 0;

    while (instr)
    {
        switch (instr->op)
        {
        case MIR_STORE:
            if (!found_store)
            {
                // First store
                found_store = 1;
            }
            else
            {
                // Final store
                found_final_store = 1;
            }
            break;

        case MIR_LOAD:
            found_load = 1;
            break;

        case MIR_ADD:
            found_add = 1;
            assert(instr->operand_count == 3);
            assert(instr->operands[1].kind == MIR_REG); // Left operand from load
            assert(instr->operands[2].kind == MIR_IMM); // Right operand is immediate
            assert(instr->operands[2].value.imm == 1);  // Right operand is 1
            break;

        // Other arithmetic operations (not used in this test)
        case MIR_SUB:
        case MIR_MUL:
        case MIR_DIV:
        case MIR_MOD:
        case MIR_NEG:
            break;

        // Control flow operations (not used in this test)
        case MIR_MOVE:
        case MIR_JUMP:
        case MIR_BRANCH:
        case MIR_CALL:
        case MIR_RET:
            break;

        // Comparison operations (not used in this test)
        case MIR_CMP_EQ:
        case MIR_CMP_NE:
        case MIR_CMP_LT:
        case MIR_CMP_LE:
        case MIR_CMP_GT:
        case MIR_CMP_GE:
            break;
        }
        instr = instr->next;
    }

    // Verify all operations were found
    assert(found_store);       // Initial store
    assert(found_load);        // Load value
    assert(found_add);         // Add operation
    assert(found_final_store); // Final store

    // Clean up
    free_zir_value(alloca);
    free_zir_value(const_val1);
    free_zir_value(store1);
    free_zir_value(load);
    free_zir_value(const_one);
    free_zir_value(add);
    free_zir_value(store2);
    free_type(i32_type);
    free_mir_function(func);
    free_translation_context(ctx);
    printf("✓ Variable update test passed\n");
}

int main(void)
{
    printf("Running ZIR to MIR memory operation tests...\n");

    test_alloca_and_store();
    test_load();
    test_variable_update();

    printf("\nAll ZIR to MIR memory operation tests passed! ✓\n");
    return 0;
}