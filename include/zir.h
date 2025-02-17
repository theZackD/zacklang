#ifndef ZIR_H
#define ZIR_H

#include "static_types.h"
#include <stdbool.h>

// Forward declarations
typedef struct ZIRFunction ZIRFunction;
typedef struct ZIRBlock ZIRBlock;
typedef struct ZIRValue ZIRValue;

// Types of ZIR instructions
typedef enum
{
    ZIR_CONST,   // Constant value
    ZIR_LOCAL,   // Local variable
    ZIR_LOAD,    // Load from variable
    ZIR_STORE,   // Store to variable
    ZIR_BINARY,  // Binary operation
    ZIR_UNARY,   // Unary operation
    ZIR_CALL,    // Function call
    ZIR_BRANCH,  // Conditional branch
    ZIR_JUMP,    // Unconditional jump
    ZIR_RETURN,  // Return from function
    ZIR_ALLOCA,  // Stack allocation
    ZIR_CONVERT, // Type conversion
    ZIR_PHI      // Phi node for SSA form
} ZIRInstrKind;

// A ZIR value represents any computed value
struct ZIRValue
{
    ZIRInstrKind kind;
    Type *type;       // Type of the value
    bool is_comptime; // Whether this value is known at compile time

    union
    {
        // For ZIR_CONST
        struct
        {
            char *literal_value;
        } constant;

        // For ZIR_LOCAL
        struct
        {
            char *name;
            bool is_mutable;
        } local;

        // For ZIR_LOAD
        struct
        {
            ZIRValue *address;
        } load;

        // For ZIR_STORE
        struct
        {
            ZIRValue *address;
            ZIRValue *value;
        } store;

        // For ZIR_BINARY
        struct
        {
            char *op;
            ZIRValue *left;
            ZIRValue *right;
        } binary;

        // For ZIR_UNARY
        struct
        {
            char *op;
            ZIRValue *operand;
        } unary;

        // For ZIR_CALL
        struct
        {
            char *func_name;
            ZIRValue **args;
            int arg_count;
        } call;

        // For ZIR_BRANCH
        struct
        {
            ZIRValue *condition;
            ZIRBlock *then_block;
            ZIRBlock *else_block;
        } branch;

        // For ZIR_JUMP
        struct
        {
            ZIRBlock *target;
        } jump;

        // For ZIR_RETURN
        struct
        {
            ZIRValue *value; // NULL for void return
        } ret;

        // For ZIR_ALLOCA
        struct
        {
            Type *alloc_type;
            bool is_mutable;
        } alloca;

        // For ZIR_CONVERT
        struct
        {
            ZIRValue *value;
            Type *target_type;
        } convert;

        // For ZIR_PHI
        struct
        {
            ZIRValue **incoming_values;
            ZIRBlock **incoming_blocks;
            int incoming_count;
        } phi;
    } data;
};

// A basic block in ZIR
struct ZIRBlock
{
    char *label;
    ZIRValue **instructions;
    int instr_count;
    int capacity;
    ZIRBlock *next; // For linear order
};

// A function in ZIR
struct ZIRFunction
{
    char *name;
    Type *return_type;
    struct
    {
        char *name;
        Type *type;
    } *params;
    int param_count;
    ZIRBlock *entry_block;
    ZIRBlock **blocks; // All blocks in the function
    int block_count;
    bool is_comptime; // Whether this is a comptime function
};

// The ZIR module containing all functions
typedef struct
{
    ZIRFunction **functions;
    int func_count;
    int capacity;
} ZIRModule;

// Creation functions
ZIRModule *create_zir_module(void);
ZIRFunction *create_zir_function(const char *name, Type *return_type, bool is_comptime);
ZIRBlock *create_zir_block(const char *label);
ZIRValue *create_zir_const(Type *type, const char *value);
ZIRValue *create_zir_local(Type *type, const char *name, bool is_mutable);
ZIRValue *create_zir_load(ZIRValue *address);
ZIRValue *create_zir_store(ZIRValue *address, ZIRValue *value);
ZIRValue *create_zir_binary(const char *op, ZIRValue *left, ZIRValue *right);
ZIRValue *create_zir_unary(const char *op, ZIRValue *operand);
ZIRValue *create_zir_call(const char *func_name, ZIRValue **args, int arg_count);

// Memory operations
ZIRValue *create_zir_alloca(Type *type, bool is_mutable);
ZIRValue *create_zir_convert(ZIRValue *value, Type *target_type);

// Control flow instructions
ZIRValue *create_zir_branch(ZIRValue *condition, ZIRBlock *then_block, ZIRBlock *else_block);
ZIRValue *create_zir_jump(ZIRBlock *target);
ZIRValue *create_zir_return(ZIRValue *value);

// Block manipulation
void zir_block_add_instr(ZIRBlock *block, ZIRValue *instr);

// Function manipulation
void zir_function_add_block(ZIRFunction *func, ZIRBlock *block);
void zir_function_add_param(ZIRFunction *func, const char *name, Type *type);

// Module manipulation
void zir_module_add_function(ZIRModule *module, ZIRFunction *func);

// Memory management
void free_zir_module(ZIRModule *module);
void free_zir_function(ZIRFunction *func);
void free_zir_block(ZIRBlock *block);
void free_zir_value(ZIRValue *value);

// Validation
bool validate_zir_module(ZIRModule *module);
bool validate_zir_function(ZIRFunction *func);
bool validate_zir_block(ZIRBlock *block);
bool validate_zir_value(ZIRValue *value);

// Phi node operations
ZIRValue *create_zir_phi(Type *type, ZIRValue **incoming_values, ZIRBlock **incoming_blocks, int incoming_count);
void zir_phi_add_incoming(ZIRValue *phi, ZIRValue *value, ZIRBlock *block);

#endif // ZIR_H