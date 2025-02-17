#include "include/mir.h"
#include "include/zir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Helper function to create a simple ZIR module for testing
static ZIRModule *create_test_zir_module(void)
{
    ZIRModule *module = create_zir_module();

    // Create a function that adds two numbers
    Type *i32_type = create_type(TYPE_I32);
    ZIRFunction *func = create_zir_function("add", i32_type, false);

    // Add parameters
    zir_function_add_param(func, "a", i32_type);
    zir_function_add_param(func, "b", i32_type);

    // Create entry block
    ZIRBlock *entry = create_zir_block("entry");

    // Create local variables for parameters
    ZIRValue *a_local = create_zir_local(i32_type, "a", false);
    ZIRValue *b_local = create_zir_local(i32_type, "b", false);

    // Add instruction
    ZIRValue *add = create_zir_binary("+", a_local, b_local);

    // Return instruction
    ZIRValue *ret = create_zir_return(add);

    // Add instructions to block
    zir_block_add_instr(entry, add);
    zir_block_add_instr(entry, ret);

    // Add block to function
    zir_function_add_block(func, entry);

    // Add function to module
    zir_module_add_function(module, func);

    return module;
}

void test_simple_function_translation(void)
{
    printf("\nTesting simple function translation...\n");

    // Create test ZIR module
    ZIRModule *zir_module = create_test_zir_module();

    // Translate to MIR
    MIRModule *mir_module = translate_zir_to_mir(zir_module);

    // Verify translation
    assert(mir_module != NULL);
    assert(mir_module->first_func != NULL);
    assert(strcmp(mir_module->first_func->name, "add") == 0);

    MIRFunction *func = mir_module->first_func;
    assert(func->param_count == 2);
    assert(func->return_type->kind == TYPE_I32);

    // Verify blocks
    assert(func->first_block != NULL);
    assert(strcmp(func->first_block->label, "entry") == 0);

    // Verify instructions
    MIRBlock *block = func->first_block;
    assert(block->first_instr != NULL);
    assert(block->first_instr->op == MIR_ADD);
    assert(block->first_instr->next != NULL);
    assert(block->first_instr->next->op == MIR_RET);

    // Clean up
    free_zir_module(zir_module);
    free_mir_module(mir_module);

    printf("✓ Simple function translation test passed\n");
}

void test_control_flow_translation(void)
{
    printf("\nTesting control flow translation...\n");

    // Create a ZIR module with control flow
    ZIRModule *module = create_zir_module();
    Type *i32_type = create_type(TYPE_I32);

    // Create max function: returns the larger of two numbers
    ZIRFunction *func = create_zir_function("max", i32_type, false);
    zir_function_add_param(func, "a", i32_type);
    zir_function_add_param(func, "b", i32_type);

    // Create blocks
    ZIRBlock *entry = create_zir_block("entry");
    ZIRBlock *then_block = create_zir_block("then");
    ZIRBlock *else_block = create_zir_block("else");
    ZIRBlock *exit_block = create_zir_block("exit");

    // Create locals
    ZIRValue *a_local = create_zir_local(i32_type, "a", false);
    ZIRValue *b_local = create_zir_local(i32_type, "b", false);

    // Create comparison
    ZIRValue *cmp = create_zir_binary(">", a_local, b_local);

    // Create branch
    ZIRValue *branch = create_zir_branch(cmp, then_block, else_block);
    zir_block_add_instr(entry, branch);

    // Then block returns a
    ZIRValue *ret_a = create_zir_return(a_local);
    zir_block_add_instr(then_block, ret_a);

    // Else block returns b
    ZIRValue *ret_b = create_zir_return(b_local);
    zir_block_add_instr(else_block, ret_b);

    // Add blocks to function
    zir_function_add_block(func, entry);
    zir_function_add_block(func, then_block);
    zir_function_add_block(func, else_block);

    // Add function to module
    zir_module_add_function(module, func);

    // Translate to MIR
    MIRModule *mir_module = translate_zir_to_mir(module);

    // Verify translation
    assert(mir_module != NULL);
    assert(mir_module->first_func != NULL);
    assert(strcmp(mir_module->first_func->name, "max") == 0);

    MIRFunction *mir_func = mir_module->first_func;
    assert(mir_func->param_count == 2);

    // Verify blocks
    assert(mir_func->first_block != NULL);
    assert(mir_func->first_block->next != NULL);
    assert(mir_func->first_block->next->next != NULL);

    // Verify branch instruction
    MIRBlock *first_block = mir_func->first_block;
    assert(first_block->first_instr != NULL);
    assert(first_block->last_instr->op == MIR_BRANCH);

    // Clean up
    free_zir_module(module);
    free_mir_module(mir_module);

    printf("✓ Control flow translation test passed\n");
}

int main(void)
{
    printf("Running ZIR to MIR translation tests...\n");

    test_simple_function_translation();
    test_control_flow_translation();

    printf("\nAll ZIR to MIR translation tests passed! ✓\n");
    return 0;
}