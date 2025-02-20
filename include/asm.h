// Assembly instruction representation
#ifndef ASM_H
#define ASM_H

#include <stdbool.h>
#include <stdint.h>

// Target architecture
typedef enum
{
    ARCH_X86_64,
    ARCH_ARM64
} Architecture;

// Register types
typedef enum
{
    REG_GENERAL, // General purpose register
    REG_FLOAT,   // Floating point register
    REG_SPECIAL  // Special registers (e.g., flags, stack pointer)
} RegisterType;

// Register structure
typedef struct
{
    int id;               // Register number
    RegisterType type;    // Type of register
    const char *name;     // Register name (architecture specific)
    bool is_caller_saved; // Whether register is caller-saved
} Register;

// Operand types
typedef enum
{
    OPERAND_NONE,
    OPERAND_REG,  // Register
    OPERAND_IMM,  // Immediate value
    OPERAND_MEM,  // Memory reference
    OPERAND_LABEL // Code label
} OperandKind;

// Memory reference structure
typedef struct
{
    Register *base;  // Base register
    Register *index; // Index register (optional)
    int scale;       // Scale for index (1, 2, 4, or 8)
    int32_t offset;  // Constant offset
} MemReference;

// Operand structure
typedef struct
{
    OperandKind kind;
    union
    {
        Register *reg;    // For OPERAND_REG
        int64_t imm;      // For OPERAND_IMM
        MemReference mem; // For OPERAND_MEM
        char *label;      // For OPERAND_LABEL
    } value;
} Operand;

// Instruction types
typedef enum
{
    // Data movement
    ASM_MOV,  // Move data
    ASM_PUSH, // Push onto stack
    ASM_POP,  // Pop from stack

    // Arithmetic
    ASM_ADD, // Add
    ASM_SUB, // Subtract
    ASM_MUL, // Multiply
    ASM_DIV, // Divide
    ASM_NEG, // Negate

    // Control flow
    ASM_CMP,  // Compare
    ASM_JMP,  // Unconditional jump
    ASM_JE,   // Jump if equal
    ASM_JNE,  // Jump if not equal
    ASM_JL,   // Jump if less
    ASM_JLE,  // Jump if less or equal
    ASM_JG,   // Jump if greater
    ASM_JGE,  // Jump if greater or equal
    ASM_CALL, // Function call
    ASM_RET,  // Return from function

    // Data movement with extension
    ASM_MOVSX, // Move with sign extension
    ASM_MOVZX  // Move with zero extension
} InstructionType;

// Instruction structure
typedef struct
{
    InstructionType type;
    Operand operands[3]; // Most instructions need at most 3 operands
    int operand_count;
    char *comment; // Optional comment for assembly output
} Instruction;

// Function to create instructions
Instruction *create_instruction(InstructionType type, int operand_count);

// Create operands
Operand create_asm_reg_operand(Register *reg);
Operand create_asm_imm_operand(int64_t value);
Operand create_asm_mem_operand(Register *base, int32_t offset);
Operand create_asm_mem_operand_indexed(Register *base, Register *index, int scale, int32_t offset);
Operand create_asm_label_operand(const char *label);

// Function to format instruction as string
char *format_instruction(Architecture arch, const Instruction *instr);

// Register allocation functions
Register *get_available_register(RegisterType type);
void free_register(Register *reg);
void spill_register(Register *reg);

// Memory management
void free_instruction(Instruction *instr);

#endif // ASM_H