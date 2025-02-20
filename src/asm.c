#include "include/asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// x86-64 register definitions
static Register x86_64_registers[] = {
    {0, REG_GENERAL, "rax", true},   // Return value
    {1, REG_GENERAL, "rbx", false},  // Callee-saved
    {2, REG_GENERAL, "rcx", true},   // 4th argument
    {3, REG_GENERAL, "rdx", true},   // 3rd argument
    {4, REG_GENERAL, "rsi", true},   // 2nd argument
    {5, REG_GENERAL, "rdi", true},   // 1st argument
    {6, REG_GENERAL, "rbp", false},  // Base pointer
    {7, REG_GENERAL, "rsp", false},  // Stack pointer
    {8, REG_GENERAL, "r8", true},    // 5th argument
    {9, REG_GENERAL, "r9", true},    // 6th argument
    {10, REG_GENERAL, "r10", true},  // Caller-saved
    {11, REG_GENERAL, "r11", true},  // Caller-saved
    {12, REG_GENERAL, "r12", false}, // Callee-saved
    {13, REG_GENERAL, "r13", false}, // Callee-saved
    {14, REG_GENERAL, "r14", false}, // Callee-saved
    {15, REG_GENERAL, "r15", false}, // Callee-saved
};

// ARM64 register definitions
static Register arm64_registers[] = {
    {0, REG_GENERAL, "x0", true},    // Return value/1st argument
    {1, REG_GENERAL, "x1", true},    // 2nd argument
    {2, REG_GENERAL, "x2", true},    // 3rd argument
    {3, REG_GENERAL, "x3", true},    // 4th argument
    {4, REG_GENERAL, "x4", true},    // 5th argument
    {5, REG_GENERAL, "x5", true},    // 6th argument
    {6, REG_GENERAL, "x6", true},    // 7th argument
    {7, REG_GENERAL, "x7", true},    // 8th argument
    {8, REG_GENERAL, "x8", true},    // Indirect result location
    {9, REG_GENERAL, "x9", true},    // Temporary register
    {10, REG_GENERAL, "x10", true},  // Temporary register
    {11, REG_GENERAL, "x11", true},  // Temporary register
    {12, REG_GENERAL, "x12", true},  // Temporary register
    {13, REG_GENERAL, "x13", true},  // Temporary register
    {14, REG_GENERAL, "x14", true},  // Temporary register
    {15, REG_GENERAL, "x15", true},  // Temporary register
    {16, REG_GENERAL, "x16", true},  // IP0 (intra-procedure-call temporary)
    {17, REG_GENERAL, "x17", true},  // IP1 (intra-procedure-call temporary)
    {18, REG_GENERAL, "x18", false}, // Platform register (reserved)
    {19, REG_GENERAL, "x19", false}, // Callee-saved
    {20, REG_GENERAL, "x20", false}, // Callee-saved
    {21, REG_GENERAL, "x21", false}, // Callee-saved
    {22, REG_GENERAL, "x22", false}, // Callee-saved
    {23, REG_GENERAL, "x23", false}, // Callee-saved
    {24, REG_GENERAL, "x24", false}, // Callee-saved
    {25, REG_GENERAL, "x25", false}, // Callee-saved
    {26, REG_GENERAL, "x26", false}, // Callee-saved
    {27, REG_GENERAL, "x27", false}, // Callee-saved
    {28, REG_GENERAL, "x28", false}, // Callee-saved
    {29, REG_GENERAL, "fp", false},  // Frame pointer
    {30, REG_GENERAL, "lr", false},  // Link register
    {31, REG_GENERAL, "sp", false},  // Stack pointer
};

// Create a new instruction
Instruction *create_instruction(InstructionType type, int operand_count)
{
    if (operand_count < 0 || operand_count > 3)
    {
        fprintf(stderr, "Invalid operand count: %d\n", operand_count);
        return NULL;
    }

    Instruction *instr = malloc(sizeof(Instruction));
    if (!instr)
    {
        fprintf(stderr, "Failed to allocate instruction\n");
        return NULL;
    }

    instr->type = type;
    instr->operand_count = operand_count;
    instr->comment = NULL;

    // Initialize operands to NONE
    for (int i = 0; i < 3; i++)
    {
        instr->operands[i].kind = OPERAND_NONE;
    }

    return instr;
}

// Create a register operand
Operand create_asm_reg_operand(Register *reg)
{
    Operand op;
    op.kind = OPERAND_REG;
    op.value.reg = reg;
    return op;
}

// Create an immediate operand
Operand create_asm_imm_operand(int64_t value)
{
    Operand op;
    op.kind = OPERAND_IMM;
    op.value.imm = value;
    return op;
}

// Create a memory operand
Operand create_asm_mem_operand(Register *base, int32_t offset)
{
    Operand op;
    op.kind = OPERAND_MEM;
    op.value.mem.base = base;
    op.value.mem.index = NULL;
    op.value.mem.scale = 0;
    op.value.mem.offset = offset;
    return op;
}

// Create an indexed memory operand
Operand create_asm_mem_operand_indexed(Register *base, Register *index, int scale, int32_t offset)
{
    Operand op;
    op.kind = OPERAND_MEM;
    op.value.mem.base = base;
    op.value.mem.index = index;
    op.value.mem.scale = scale;
    op.value.mem.offset = offset;
    return op;
}

// Create a label operand
Operand create_asm_label_operand(const char *label)
{
    Operand op;
    op.kind = OPERAND_LABEL;
    op.value.label = strdup(label);
    return op;
}

// Format memory reference
static char *format_mem_reference(Architecture arch, const MemReference *mem)
{
    char buffer[128];
    const char *base_name = mem->base ? mem->base->name : "";

    if (arch == ARCH_X86_64)
    {
        if (mem->index)
        {
            snprintf(buffer, sizeof(buffer), "[%s + %s * %d + %d]",
                     base_name, mem->index->name, mem->scale, mem->offset);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "[%s + %d]",
                     base_name, mem->offset);
        }
    }
    else
    { // ARM64
        if (mem->index)
        {
            snprintf(buffer, sizeof(buffer), "[%s, %s, LSL #%d]#%d",
                     base_name, mem->index->name, mem->scale, mem->offset);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "[%s]#%d",
                     base_name, mem->offset);
        }
    }

    return strdup(buffer);
}

// Format operand
static char *format_operand(Architecture arch, const Operand *op)
{
    char buffer[128];

    switch (op->kind)
    {
    case OPERAND_NONE:
        return strdup("");

    case OPERAND_REG:
        return strdup(op->value.reg->name);

    case OPERAND_IMM:
        snprintf(buffer, sizeof(buffer), "%lld", op->value.imm);
        return strdup(buffer);

    case OPERAND_MEM:
        return format_mem_reference(arch, &op->value.mem);

    case OPERAND_LABEL:
        return strdup(op->value.label);

    default:
        fprintf(stderr, "Unknown operand kind: %d\n", op->kind);
        return strdup("???");
    }
}

// Format instruction as string
char *format_instruction(Architecture arch, const Instruction *instr)
{
    if (!instr)
        return NULL;

    // Buffer for the complete instruction
    char buffer[256];
    int pos = 0;

    // Get instruction mnemonic
    const char *mnemonic = "";
    switch (instr->type)
    {
    case ASM_MOV:
        mnemonic = (arch == ARCH_X86_64) ? "mov" : "mov";
        break;
    case ASM_PUSH:
        mnemonic = (arch == ARCH_X86_64) ? "push" : "str";
        break;
    case ASM_POP:
        mnemonic = (arch == ARCH_X86_64) ? "pop" : "ldr";
        break;
    case ASM_ADD:
        mnemonic = "add";
        break;
    case ASM_SUB:
        mnemonic = "sub";
        break;
    case ASM_MUL:
        mnemonic = (arch == ARCH_X86_64) ? "imul" : "mul";
        break;
    case ASM_DIV:
        mnemonic = (arch == ARCH_X86_64) ? "idiv" : "sdiv";
        break;
    case ASM_NEG:
        mnemonic = "neg";
        break;
    case ASM_CMP:
        mnemonic = "cmp";
        break;
    case ASM_JMP:
        mnemonic = (arch == ARCH_X86_64) ? "jmp" : "b";
        break;
    case ASM_JE:
        mnemonic = (arch == ARCH_X86_64) ? "je" : "b.eq";
        break;
    case ASM_JNE:
        mnemonic = (arch == ARCH_X86_64) ? "jne" : "b.ne";
        break;
    case ASM_JL:
        mnemonic = (arch == ARCH_X86_64) ? "jl" : "b.lt";
        break;
    case ASM_JLE:
        mnemonic = (arch == ARCH_X86_64) ? "jle" : "b.le";
        break;
    case ASM_JG:
        mnemonic = (arch == ARCH_X86_64) ? "jg" : "b.gt";
        break;
    case ASM_JGE:
        mnemonic = (arch == ARCH_X86_64) ? "jge" : "b.ge";
        break;
    case ASM_CALL:
        mnemonic = (arch == ARCH_X86_64) ? "call" : "bl";
        break;
    case ASM_RET:
        mnemonic = (arch == ARCH_X86_64) ? "ret" : "ret";
        break;
    case ASM_MOVSX:
        mnemonic = (arch == ARCH_X86_64) ? "movsx" : "sxtw";
        break;
    case ASM_MOVZX:
        mnemonic = (arch == ARCH_X86_64) ? "movzx" : "uxtw";
        break;
    default:
        mnemonic = "???";
        break;
    }

    // Add mnemonic
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "    %s", mnemonic);

    // Add operands
    for (int i = 0; i < instr->operand_count; i++)
    {
        char *op_str = format_operand(arch, &instr->operands[i]);
        pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                        i == 0 ? " %s" : ", %s", op_str);
        free(op_str);
    }

    // Add comment if present
    if (instr->comment)
    {
        pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                        " # %s", instr->comment);
    }

    return strdup(buffer);
}

// Memory management
void free_instruction(Instruction *instr)
{
    if (!instr)
        return;

    // Free any label operands
    for (int i = 0; i < instr->operand_count; i++)
    {
        if (instr->operands[i].kind == OPERAND_LABEL)
        {
            free(instr->operands[i].value.label);
        }
    }

    free(instr->comment);
    free(instr);
}

// Register allocation (simple implementation for now)
static bool register_in_use[32] = {false}; // Tracks register usage

Register *get_available_register(RegisterType type)
{
    Register *regs = x86_64_registers; // Default to x86-64
    int reg_count = sizeof(x86_64_registers) / sizeof(Register);

    for (int i = 0; i < reg_count; i++)
    {
        if (regs[i].type == type && !register_in_use[i])
        {
            register_in_use[i] = true;
            return &regs[i];
        }
    }

    return NULL; // No available register
}

void free_register(Register *reg)
{
    if (!reg)
        return;
    register_in_use[reg->id] = false;
}

void spill_register(Register *reg)
{
    // TODO: Implement register spilling
    fprintf(stderr, "Register spilling not implemented yet\n");
}