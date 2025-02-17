#ifndef MIR_H
#define MIR_H

#include "static_types.h"
#include "zir.h"
#include <stdbool.h>
#include <stdint.h>

// Special registers
#define FRAME_POINTER_REG 0 // Register used as frame pointer

// MIR instruction opcodes
typedef enum
{
    MIR_ADD,    // Add two values
    MIR_SUB,    // Subtract
    MIR_MUL,    // Multiply
    MIR_DIV,    // Divide
    MIR_MOD,    // Modulo
    MIR_NEG,    // Negate
    MIR_LOAD,   // Load from memory
    MIR_STORE,  // Store to memory
    MIR_MOVE,   // Move between registers
    MIR_JUMP,   // Unconditional jump
    MIR_BRANCH, // Conditional branch
    MIR_CALL,   // Function call
    MIR_RET,    // Return
    MIR_CMP_EQ, // Compare equal
    MIR_CMP_NE, // Compare not equal
    MIR_CMP_LT, // Compare less than
    MIR_CMP_LE, // Compare less equal
    MIR_CMP_GT, // Compare greater than
    MIR_CMP_GE, // Compare greater equal
    MIR_PHI     // Phi node for SSA form
} MIROpcode;

// MIR operand types
typedef enum
{
    MIR_REG,  // Virtual register
    MIR_IMM,  // Immediate value
    MIR_MEM,  // Memory location
    MIR_LABEL // Code label
} MIROperandKind;

// MIR operand structure
typedef struct
{
    MIROperandKind kind;
    Type *type; // Type of the operand
    union
    {
        int reg_num; // Register number
        int64_t imm; // Immediate value
        char *label; // Label name
        struct
        {
            int base_reg; // Base register for memory access
            int offset;   // Offset from base
        } mem;
    } value;
} MIROperand;

// MIR instruction
typedef struct MIRInstr
{
    MIROpcode op;
    MIROperand *operands;
    int operand_count;
    struct MIRInstr *next;
} MIRInstr;

// MIR basic block
typedef struct MIRBlock
{
    char *label;
    MIRInstr *first_instr;
    MIRInstr *last_instr;
    struct MIRBlock *next;
} MIRBlock;

// MIR function
typedef struct MIRFunction
{
    char *name;
    int reg_count;     // Number of virtual registers needed
    Type *return_type; // Function return type
    struct
    {
        char *name;
        Type *type;
    } *params; // Array of parameters
    int param_count;
    MIRBlock *first_block;
    MIRBlock *last_block;
    struct MIRFunction *next;
} MIRFunction;

// MIR module
typedef struct
{
    MIRFunction *first_func;
    MIRFunction *last_func;
} MIRModule;

// Creation functions
MIRModule *create_mir_module(void);
MIRFunction *create_mir_function(const char *name, Type *return_type);
MIRBlock *create_mir_block(const char *label);
MIRInstr *create_mir_instr(MIROpcode op, MIROperand *operands, int operand_count);
MIROperand create_reg_operand(int reg_num, Type *type);
MIROperand create_imm_operand(int64_t value, Type *type);
MIROperand create_label_operand(const char *label);
MIROperand create_mem_operand(int base_reg, int offset, Type *type);

// Phi node specific functions
MIRInstr *create_mir_phi(MIROperand result, MIROperand *values, MIROperand *blocks, int count);
void mir_phi_add_incoming(MIRInstr *phi, MIROperand value, MIROperand block);

// Module manipulation
void mir_module_add_function(MIRModule *module, MIRFunction *func);

// Function manipulation
void mir_function_add_block(MIRFunction *func, MIRBlock *block);
void mir_function_add_param(MIRFunction *func, const char *name, Type *type);

// Block manipulation
void mir_block_add_instr(MIRBlock *block, MIRInstr *instr);

// Memory management
void free_mir_module(MIRModule *module);
void free_mir_function(MIRFunction *func);
void free_mir_block(MIRBlock *block);
void free_mir_instr(MIRInstr *instr);

// Validation
bool validate_mir_module(MIRModule *module);
bool validate_mir_function(MIRFunction *func);
bool validate_mir_block(MIRBlock *block);
bool validate_mir_instr(MIRInstr *instr);

// Main translation function
MIRModule *translate_zir_to_mir(ZIRModule *zir_module);

#endif // MIR_H