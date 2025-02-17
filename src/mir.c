#include "include/mir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function for memory allocation with error checking
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

// Helper function for string duplication with error checking
static char *safe_strdup(const char *str)
{
    if (!str)
        return NULL;
    char *dup = strdup(str);
    if (!dup)
    {
        fprintf(stderr, "String duplication failed\n");
        exit(1);
    }
    return dup;
}

// Creation functions
MIRModule *create_mir_module(void)
{
    MIRModule *module = safe_malloc(sizeof(MIRModule));
    module->first_func = NULL;
    module->last_func = NULL;
    return module;
}

MIRFunction *create_mir_function(const char *name, Type *return_type)
{
    MIRFunction *func = safe_malloc(sizeof(MIRFunction));
    func->name = safe_strdup(name);
    func->return_type = return_type;
    func->reg_count = 0;
    func->params = NULL;
    func->param_count = 0;
    func->first_block = NULL;
    func->last_block = NULL;
    func->next = NULL;
    return func;
}

MIRBlock *create_mir_block(const char *label)
{
    MIRBlock *block = safe_malloc(sizeof(MIRBlock));
    block->label = safe_strdup(label);
    block->first_instr = NULL;
    block->last_instr = NULL;
    block->next = NULL;
    return block;
}

MIRInstr *create_mir_instr(MIROpcode op, MIROperand *operands, int operand_count)
{
    MIRInstr *instr = safe_malloc(sizeof(MIRInstr));
    instr->op = op;
    instr->operand_count = operand_count;
    instr->operands = safe_malloc(operand_count * sizeof(MIROperand));
    memcpy(instr->operands, operands, operand_count * sizeof(MIROperand));
    instr->next = NULL;
    return instr;
}

// Create a phi instruction
MIRInstr *create_mir_phi(MIROperand result, MIROperand *values, MIROperand *blocks, int count)
{
    // Calculate total operands: result + count*(value + block)
    int total_operands = 1 + (count * 2);
    MIROperand *operands = safe_malloc(total_operands * sizeof(MIROperand));

    // Set result operand
    operands[0] = result;

    // Copy incoming values and blocks
    if (count > 0 && values && blocks)
    {
        for (int i = 0; i < count; i++)
        {
            // Deep copy operands
            operands[1 + i * 2] = values[i];
            if (values[i].kind == MIR_LABEL)
            {
                operands[1 + i * 2].value.label = safe_strdup(values[i].value.label);
            }

            operands[2 + i * 2] = blocks[i];
            if (blocks[i].kind == MIR_LABEL)
            {
                operands[2 + i * 2].value.label = safe_strdup(blocks[i].value.label);
            }
        }
    }

    return create_mir_instr(MIR_PHI, operands, total_operands);
}

// Operand creation functions
MIROperand create_reg_operand(int reg_num, Type *type)
{
    MIROperand op = {0};
    op.kind = MIR_REG;
    op.type = type;
    op.value.reg_num = reg_num;
    return op;
}

MIROperand create_imm_operand(int64_t value, Type *type)
{
    MIROperand op = {0};
    op.kind = MIR_IMM;
    op.type = type;
    op.value.imm = value;
    return op;
}

MIROperand create_label_operand(const char *label)
{
    MIROperand op = {0};
    op.kind = MIR_LABEL;
    op.type = NULL;
    op.value.label = safe_strdup(label);
    return op;
}

MIROperand create_mem_operand(int base_reg, int offset, Type *type)
{
    MIROperand op = {0};
    op.kind = MIR_MEM;
    op.type = type;
    op.value.mem.base_reg = base_reg;
    op.value.mem.offset = offset;
    return op;
}

// Module manipulation
void mir_module_add_function(MIRModule *module, MIRFunction *func)
{
    if (!module->first_func)
    {
        module->first_func = func;
    }
    else
    {
        module->last_func->next = func;
    }
    module->last_func = func;
}

// Function manipulation
void mir_function_add_block(MIRFunction *func, MIRBlock *block)
{
    if (!func->first_block)
    {
        func->first_block = block;
    }
    else
    {
        func->last_block->next = block;
    }
    func->last_block = block;
}

void mir_function_add_param(MIRFunction *func, const char *name, Type *type)
{
    func->param_count++;
    func->params = realloc(func->params, func->param_count * sizeof(*func->params));
    if (!func->params)
    {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(1);
    }
    func->params[func->param_count - 1].name = safe_strdup(name);
    func->params[func->param_count - 1].type = type;
}

// Block manipulation
void mir_block_add_instr(MIRBlock *block, MIRInstr *instr)
{
    if (!block->first_instr)
    {
        block->first_instr = instr;
    }
    else
    {
        block->last_instr->next = instr;
    }
    block->last_instr = instr;
}

// Add an incoming value to a phi instruction
void mir_phi_add_incoming(MIRInstr *phi, MIROperand value, MIROperand block)
{
    if (!phi || phi->op != MIR_PHI)
        return;

    // Calculate new operand count
    int old_count = (phi->operand_count - 1) / 2; // Subtract result, divide remaining by 2
    int new_count = phi->operand_count + 2;       // Add space for new value and block

    // Reallocate operands array
    phi->operands = realloc(phi->operands, new_count * sizeof(MIROperand));
    if (!phi->operands)
    {
        fprintf(stderr, "Memory reallocation failed for phi node\n");
        exit(1);
    }

    // Add new value and block (deep copy)
    phi->operands[new_count - 2] = value;
    if (value.kind == MIR_LABEL)
    {
        phi->operands[new_count - 2].value.label = safe_strdup(value.value.label);
    }

    phi->operands[new_count - 1] = block;
    if (block.kind == MIR_LABEL)
    {
        phi->operands[new_count - 1].value.label = safe_strdup(block.value.label);
    }

    phi->operand_count = new_count;
}

// Memory management
void free_mir_instr(MIRInstr *instr)
{
    if (!instr)
        return;

    // Free operands array
    if (instr->operands)
    {
        // For each operand that might contain allocated memory
        for (int i = 0; i < instr->operand_count; i++)
        {
            if (instr->operands[i].kind == MIR_LABEL && instr->operands[i].value.label)
            {
                free(instr->operands[i].value.label);
            }
        }
        free(instr->operands);
    }

    free(instr);
}

void free_mir_block(MIRBlock *block)
{
    if (!block)
        return;
    MIRInstr *instr = block->first_instr;
    while (instr)
    {
        MIRInstr *next = instr->next;
        free_mir_instr(instr);
        instr = next;
    }
    free(block->label);
    free(block);
}

void free_mir_function(MIRFunction *func)
{
    if (!func)
        return;
    for (int i = 0; i < func->param_count; i++)
    {
        free(func->params[i].name);
    }
    free(func->params);
    MIRBlock *block = func->first_block;
    while (block)
    {
        MIRBlock *next = block->next;
        free_mir_block(block);
        block = next;
    }
    free(func->name);
    free(func);
}

void free_mir_module(MIRModule *module)
{
    if (!module)
        return;
    MIRFunction *func = module->first_func;
    while (func)
    {
        MIRFunction *next = func->next;
        free_mir_function(func);
        func = next;
    }
    free(module);
}

// Validation functions
bool validate_mir_instr(MIRInstr *instr)
{
    if (!instr)
        return false;
    if (!instr->operands && instr->operand_count > 0)
        return false;

    // Validate operand count based on opcode
    switch (instr->op)
    {
    case MIR_ADD:
    case MIR_SUB:
    case MIR_MUL:
    case MIR_DIV:
    case MIR_MOD:
    case MIR_CMP_EQ:
    case MIR_CMP_NE:
    case MIR_CMP_LT:
    case MIR_CMP_LE:
    case MIR_CMP_GT:
    case MIR_CMP_GE:
        return instr->operand_count == 3; // dest, src1, src2

    case MIR_NEG:
    case MIR_LOAD:
    case MIR_MOVE:
        return instr->operand_count == 2; // dest, src

    case MIR_STORE:
        return instr->operand_count == 2; // dest_addr, src

    case MIR_JUMP:
        return instr->operand_count == 1 && instr->operands[0].kind == MIR_LABEL;

    case MIR_BRANCH:
        return instr->operand_count == 3 && instr->operands[1].kind == MIR_LABEL && instr->operands[2].kind == MIR_LABEL;

    case MIR_RET:
        return instr->operand_count <= 1;

    case MIR_PHI:
    {
        // Must have at least a result operand
        if (instr->operand_count < 1)
            return false;

        // Result must be a register
        if (instr->operands[0].kind != MIR_REG)
            return false;

        // Remaining operands must come in pairs (value + block)
        if ((instr->operand_count - 1) % 2 != 0)
            return false;

        // Check each value/block pair
        for (int i = 1; i < instr->operand_count; i += 2)
        {
            // Value must be register or immediate
            if (instr->operands[i].kind != MIR_REG &&
                instr->operands[i].kind != MIR_IMM)
            {
                return false;
            }

            // Block must be a label
            if (instr->operands[i + 1].kind != MIR_LABEL)
            {
                return false;
            }

            // Value type must match result type
            if (!types_are_equal(instr->operands[i].type,
                                 instr->operands[0].type))
            {
                return false;
            }
        }
        return true;
    }

    default:
        return false;
    }
}

bool validate_mir_block(MIRBlock *block)
{
    if (!block || !block->label)
        return false;

    MIRInstr *instr = block->first_instr;
    while (instr)
    {
        if (!validate_mir_instr(instr))
            return false;
        instr = instr->next;
    }
    return true;
}

bool validate_mir_function(MIRFunction *func)
{
    if (!func || !func->name || !func->return_type)
        return false;

    // Validate parameters
    for (int i = 0; i < func->param_count; i++)
    {
        if (!func->params[i].name || !func->params[i].type)
            return false;
    }

    // Validate blocks
    MIRBlock *block = func->first_block;
    while (block)
    {
        if (!validate_mir_block(block))
            return false;
        block = block->next;
    }
    return true;
}

bool validate_mir_module(MIRModule *module)
{
    if (!module)
        return false;

    MIRFunction *func = module->first_func;
    while (func)
    {
        if (!validate_mir_function(func))
            return false;
        func = func->next;
    }
    return true;
}