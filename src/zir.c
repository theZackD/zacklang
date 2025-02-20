#include "include/zir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8
#define INITIAL_BLOCK_EDGES_CAPACITY 4

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

// Module creation and manipulation
ZIRModule *create_zir_module(void)
{
    ZIRModule *module = safe_malloc(sizeof(ZIRModule));
    module->functions = safe_malloc(INITIAL_CAPACITY * sizeof(ZIRFunction *));
    module->func_count = 0;
    module->capacity = INITIAL_CAPACITY;
    return module;
}

void zir_module_add_function(ZIRModule *module, ZIRFunction *func)
{
    if (module->func_count >= module->capacity)
    {
        module->capacity *= 2;
        ZIRFunction **new_funcs = realloc(module->functions,
                                          module->capacity * sizeof(ZIRFunction *));
        if (!new_funcs)
        {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
        module->functions = new_funcs;
    }
    module->functions[module->func_count++] = func;
}

// Function creation and manipulation
ZIRFunction *create_zir_function(const char *name, Type *return_type, bool is_comptime)
{
    ZIRFunction *func = safe_malloc(sizeof(ZIRFunction));
    func->name = safe_strdup(name);
    func->return_type = return_type;
    func->is_comptime = is_comptime;
    func->params = NULL;
    func->param_count = 0;
    func->entry_block = NULL;
    func->blocks = safe_malloc(INITIAL_CAPACITY * sizeof(ZIRBlock *));
    func->block_count = 0;
    return func;
}

void zir_function_add_param(ZIRFunction *func, const char *name, Type *type)
{
    func->param_count++;
    func->params = realloc(func->params,
                           func->param_count * sizeof(*func->params));
    if (!func->params)
    {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(1);
    }
    func->params[func->param_count - 1].name = safe_strdup(name);
    func->params[func->param_count - 1].type = type;
}

void zir_function_add_block(ZIRFunction *func, ZIRBlock *block)
{
    if (!func->entry_block)
    {
        func->entry_block = block;
    }
    if (func->block_count > 0)
    {
        ZIRBlock *last = func->blocks[func->block_count - 1];
        last->next = block;
    }
    func->blocks[func->block_count++] = block;
}

// Block creation and manipulation
ZIRBlock *create_zir_block(const char *label)
{
    ZIRBlock *block = safe_malloc(sizeof(ZIRBlock));
    block->label = safe_strdup(label);
    block->instructions = safe_malloc(INITIAL_CAPACITY * sizeof(ZIRValue *));
    block->instr_count = 0;
    block->capacity = INITIAL_CAPACITY;
    block->next = NULL;

    // Initialize predecessor tracking
    block->predecessors = safe_malloc(INITIAL_BLOCK_EDGES_CAPACITY * sizeof(ZIRBlock *));
    block->pred_count = 0;
    block->pred_capacity = INITIAL_BLOCK_EDGES_CAPACITY;

    // Initialize successor tracking
    block->successors = safe_malloc(INITIAL_BLOCK_EDGES_CAPACITY * sizeof(ZIRBlock *));
    block->succ_count = 0;
    block->succ_capacity = INITIAL_BLOCK_EDGES_CAPACITY;

    return block;
}

void zir_block_add_instr(ZIRBlock *block, ZIRValue *instr)
{
    if (!block || !instr)
        return;

    // Add the instruction
    if (block->instr_count >= block->capacity)
    {
        block->capacity *= 2;
        ZIRValue **new_instrs = realloc(block->instructions,
                                        block->capacity * sizeof(ZIRValue *));
        if (!new_instrs)
        {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
        block->instructions = new_instrs;
    }
    block->instructions[block->instr_count++] = instr;

    // Update control flow information based on terminator instructions
    if (instr->kind == ZIR_BRANCH)
    {
        // Add successors for branch instruction
        zir_block_add_successor(block, instr->data.branch.then_block);
        if (instr->data.branch.else_block)
        {
            zir_block_add_successor(block, instr->data.branch.else_block);
        }
    }
    else if (instr->kind == ZIR_JUMP)
    {
        // Add successor for jump instruction
        zir_block_add_successor(block, instr->data.jump.target);
    }
}

// Value creation functions
static ZIRValue *create_zir_value(ZIRInstrKind kind, Type *type)
{
    ZIRValue *value = safe_malloc(sizeof(ZIRValue));
    value->kind = kind;
    value->type = type;
    value->is_comptime = false; // Default to runtime value
    return value;
}

ZIRValue *create_zir_const(Type *type, const char *value)
{
    ZIRValue *val = create_zir_value(ZIR_CONST, type);
    val->is_comptime = true; // Constants are always comptime
    val->data.constant.literal_value = safe_strdup(value);
    return val;
}

ZIRValue *create_zir_local(Type *type, const char *name, bool is_mutable)
{
    ZIRValue *val = create_zir_value(ZIR_LOCAL, type);
    val->data.local.name = safe_strdup(name);
    val->data.local.is_mutable = is_mutable;
    return val;
}

ZIRValue *create_zir_load(ZIRValue *address)
{
    ZIRValue *val = create_zir_value(ZIR_LOAD, address->type);
    val->data.load.address = address;
    val->is_comptime = address->is_comptime;
    return val;
}

ZIRValue *create_zir_store(ZIRValue *address, ZIRValue *value)
{
    if (!address || !value)
        return NULL;
    ZIRValue *val = create_zir_value(ZIR_STORE, value->type);
    val->data.store.address = address;
    val->data.store.value = value;
    return val;
}

ZIRValue *create_zir_binary(const char *op, ZIRValue *left, ZIRValue *right)
{
    if (!left || !right)
        return NULL;

    // Get result type from binary operation
    Type *result_type = get_binary_op_type(op, left->type, right->type);
    ZIRValue *value = create_zir_value(ZIR_BINARY, result_type);
    value->data.binary.op = safe_strdup(op);
    value->data.binary.left = left;
    value->data.binary.right = right;

    // Propagate comptime: result is comptime if both operands are comptime
    value->is_comptime = left->is_comptime && right->is_comptime;

    return value;
}

ZIRValue *create_zir_unary(const char *op, ZIRValue *operand)
{
    if (!operand)
        return NULL;
    Type *result_type = get_unary_op_type(op, operand->type);
    ZIRValue *val = create_zir_value(ZIR_UNARY, result_type);
    val->data.unary.op = safe_strdup(op);
    val->data.unary.operand = operand;
    val->is_comptime = operand->is_comptime;
    return val;
}

ZIRValue *create_zir_call(const char *func_name, ZIRValue **args, int arg_count)
{
    // Note: Return type will need to be set after creation based on function lookup
    ZIRValue *val = create_zir_value(ZIR_CALL, NULL);
    val->data.call.func_name = safe_strdup(func_name);
    val->data.call.args = safe_malloc(arg_count * sizeof(ZIRValue *));
    memcpy(val->data.call.args, args, arg_count * sizeof(ZIRValue *));
    val->data.call.arg_count = arg_count;

    // A call is comptime only if the function and all arguments are comptime
    val->is_comptime = true; // Start optimistic
    for (int i = 0; i < arg_count; i++)
    {
        if (!args[i]->is_comptime)
        {
            val->is_comptime = false;
            break;
        }
    }
    return val;
}

ZIRValue *create_zir_branch(ZIRValue *condition, ZIRBlock *then_block, ZIRBlock *else_block)
{
    if (!condition || !then_block)
        return NULL;

    // Branch instructions don't have a meaningful type
    ZIRValue *val = create_zir_value(ZIR_BRANCH, condition->type);
    val->data.branch.condition = condition;
    val->data.branch.then_block = then_block;
    val->data.branch.else_block = else_block;

    // Update control flow information
    // This will be done when the branch is added to a block via zir_block_add_instr

    return val;
}

ZIRValue *create_zir_jump(ZIRBlock *target)
{
    if (!target)
        return NULL;

    // Jump instructions don't have a meaningful type
    ZIRValue *val = create_zir_value(ZIR_JUMP, NULL);
    val->data.jump.target = target;

    // Update control flow information
    // This will be done when the jump is added to a block via zir_block_add_instr

    return val;
}

ZIRValue *create_zir_return(ZIRValue *value)
{
    // Return instructions take on the type of their value, or void
    Type *type = value ? value->type : NULL;
    ZIRValue *val = create_zir_value(ZIR_RETURN, type);
    val->data.ret.value = value;
    return val;
}

ZIRValue *create_zir_alloca(Type *type, bool is_mutable)
{
    ZIRValue *val = create_zir_value(ZIR_ALLOCA, type);
    val->data.alloca.alloc_type = type;
    val->data.alloca.is_mutable = is_mutable;
    return val;
}

ZIRValue *create_zir_convert(ZIRValue *value, Type *target_type)
{
    if (!value || !target_type)
        return NULL;
    ZIRValue *val = create_zir_value(ZIR_CONVERT, target_type);
    val->data.convert.value = value;
    val->data.convert.target_type = target_type;
    val->is_comptime = value->is_comptime;
    return val;
}

ZIRValue *create_zir_phi(Type *type, ZIRValue **incoming_values, ZIRBlock **incoming_blocks, int incoming_count)
{
    if (!type || incoming_count < 0)
        return NULL;

    ZIRValue *val = create_zir_value(ZIR_PHI, type);

    // Initialize with empty arrays if no incoming values provided
    if (incoming_count == 0)
    {
        val->data.phi.incoming_values = NULL;
        val->data.phi.incoming_blocks = NULL;
        val->data.phi.incoming_count = 0;
        return val;
    }

    // Allocate and copy incoming values and blocks
    val->data.phi.incoming_values = safe_malloc(incoming_count * sizeof(ZIRValue *));
    val->data.phi.incoming_blocks = safe_malloc(incoming_count * sizeof(ZIRBlock *));

    if (incoming_values && incoming_blocks)
    {
        memcpy(val->data.phi.incoming_values, incoming_values, incoming_count * sizeof(ZIRValue *));
        memcpy(val->data.phi.incoming_blocks, incoming_blocks, incoming_count * sizeof(ZIRBlock *));
    }

    val->data.phi.incoming_count = incoming_count;

    // A phi is comptime if all incoming values are comptime
    val->is_comptime = true;
    for (int i = 0; i < incoming_count; i++)
    {
        if (incoming_values && incoming_values[i] && !incoming_values[i]->is_comptime)
        {
            val->is_comptime = false;
            break;
        }
    }

    return val;
}

// Add an incoming value to a phi node
void zir_phi_add_incoming(ZIRValue *phi, ZIRValue *value, ZIRBlock *block)
{
    if (!phi || !value || !block || phi->kind != ZIR_PHI)
        return;

    // Extend arrays
    int new_count = phi->data.phi.incoming_count + 1;
    phi->data.phi.incoming_values = realloc(phi->data.phi.incoming_values,
                                            new_count * sizeof(ZIRValue *));
    phi->data.phi.incoming_blocks = realloc(phi->data.phi.incoming_blocks,
                                            new_count * sizeof(ZIRBlock *));

    if (!phi->data.phi.incoming_values || !phi->data.phi.incoming_blocks)
    {
        fprintf(stderr, "Memory reallocation failed for phi node\n");
        exit(1);
    }

    // Add new incoming value and block
    phi->data.phi.incoming_values[phi->data.phi.incoming_count] = value;
    phi->data.phi.incoming_blocks[phi->data.phi.incoming_count] = block;
    phi->data.phi.incoming_count = new_count;

    // Update comptime status
    if (!value->is_comptime)
    {
        phi->is_comptime = false;
    }
}

// Memory management
void free_zir_value(ZIRValue *value)
{
    if (!value)
        return;

    switch (value->kind)
    {
    case ZIR_CONST:
        free(value->data.constant.literal_value);
        break;
    case ZIR_LOCAL:
        free(value->data.local.name);
        break;
    case ZIR_BINARY:
        free(value->data.binary.op);
        break;
    case ZIR_UNARY:
        free(value->data.unary.op);
        break;
    case ZIR_CALL:
        free(value->data.call.func_name);
        free(value->data.call.args);
        break;
    case ZIR_PHI:
        free(value->data.phi.incoming_values);
        free(value->data.phi.incoming_blocks);
        break;
    default:
        break;
    }
    free(value);
}

void free_zir_block(ZIRBlock *block)
{
    if (!block)
        return;
    free(block->label);
    for (int i = 0; i < block->instr_count; i++)
    {
        free_zir_value(block->instructions[i]);
    }
    free(block->instructions);
    free(block->predecessors);
    free(block->successors);
    free(block);
}

void free_zir_function(ZIRFunction *func)
{
    if (!func)
        return;
    free(func->name);
    for (int i = 0; i < func->param_count; i++)
    {
        free(func->params[i].name);
    }
    free(func->params);
    for (int i = 0; i < func->block_count; i++)
    {
        free_zir_block(func->blocks[i]);
    }
    free(func->blocks);
    free(func);
}

void free_zir_module(ZIRModule *module)
{
    if (!module)
        return;
    for (int i = 0; i < module->func_count; i++)
    {
        free_zir_function(module->functions[i]);
    }
    free(module->functions);
    free(module);
}

// Validation functions
bool validate_zir_value(ZIRValue *value)
{
    if (!value)
        return false;
    if (!value->type && value->kind != ZIR_JUMP &&
        value->kind != ZIR_BRANCH && value->kind != ZIR_RETURN)
    {
        return false;
    }

    switch (value->kind)
    {
    case ZIR_CONST:
        return value->data.constant.literal_value != NULL;
    case ZIR_LOCAL:
        return value->data.local.name != NULL;
    case ZIR_LOAD:
        return value->data.load.address != NULL;
    case ZIR_STORE:
        return value->data.store.address != NULL &&
               value->data.store.value != NULL;
    case ZIR_BINARY:
        return value->data.binary.op != NULL &&
               value->data.binary.left != NULL &&
               value->data.binary.right != NULL;
    case ZIR_UNARY:
        return value->data.unary.op != NULL &&
               value->data.unary.operand != NULL;
    case ZIR_CALL:
        return value->data.call.func_name != NULL &&
               (value->data.call.arg_count == 0 ||
                value->data.call.args != NULL);
    case ZIR_BRANCH:
        return value->data.branch.condition != NULL &&
               value->data.branch.then_block != NULL;
    case ZIR_JUMP:
        return value->data.jump.target != NULL;
    case ZIR_PHI:
        // Phi node must have a type
        if (!value->type)
            return false;

        // Must have matching counts of values and blocks
        if (value->data.phi.incoming_count > 0)
        {
            if (!value->data.phi.incoming_values ||
                !value->data.phi.incoming_blocks)
            {
                return false;
            }

            // Check that all incoming values have compatible types
            for (int i = 0; i < value->data.phi.incoming_count; i++)
            {
                if (!value->data.phi.incoming_values[i] ||
                    !value->data.phi.incoming_blocks[i])
                {
                    return false;
                }

                // All incoming values must have the same type as the phi
                if (!types_are_equal(value->type,
                                     value->data.phi.incoming_values[i]->type))
                {
                    return false;
                }
            }
        }
        return true;
    default:
        return true;
    }
}

bool validate_zir_block(ZIRBlock *block)
{
    if (!block || !block->label)
        return false;

    for (int i = 0; i < block->instr_count; i++)
    {
        if (!validate_zir_value(block->instructions[i]))
        {
            return false;
        }
    }
    return true;
}

bool validate_zir_function(ZIRFunction *func)
{
    if (!func || !func->name || !func->return_type)
        return false;

    // Validate parameters
    for (int i = 0; i < func->param_count; i++)
    {
        if (!func->params[i].name || !func->params[i].type)
        {
            return false;
        }
    }

    // Validate blocks
    if (!func->entry_block)
        return false;
    for (int i = 0; i < func->block_count; i++)
    {
        if (!validate_zir_block(func->blocks[i]))
        {
            return false;
        }
    }
    return true;
}

bool validate_zir_module(ZIRModule *module)
{
    if (!module)
        return false;

    for (int i = 0; i < module->func_count; i++)
    {
        if (!validate_zir_function(module->functions[i]))
        {
            return false;
        }
    }
    return true;
}

// Add a predecessor to a block
void zir_block_add_predecessor(ZIRBlock *block, ZIRBlock *pred)
{
    if (!block || !pred || zir_block_has_predecessor(block, pred))
        return;

    // Resize if needed
    if (block->pred_count >= block->pred_capacity)
    {
        block->pred_capacity *= 2;
        block->predecessors = realloc(block->predecessors,
                                      block->pred_capacity * sizeof(ZIRBlock *));
        if (!block->predecessors)
        {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
    }

    block->predecessors[block->pred_count++] = pred;

    // Add this block as a successor to pred if not already present
    if (!zir_block_has_successor(pred, block))
    {
        zir_block_add_successor(pred, block);
    }
}

// Add a successor to a block
void zir_block_add_successor(ZIRBlock *block, ZIRBlock *succ)
{
    if (!block || !succ || zir_block_has_successor(block, succ))
        return;

    // Resize if needed
    if (block->succ_count >= block->succ_capacity)
    {
        block->succ_capacity *= 2;
        block->successors = realloc(block->successors,
                                    block->succ_capacity * sizeof(ZIRBlock *));
        if (!block->successors)
        {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
    }

    block->successors[block->succ_count++] = succ;

    // Add this block as a predecessor to succ if not already present
    if (!zir_block_has_predecessor(succ, block))
    {
        zir_block_add_predecessor(succ, block);
    }
}

// Remove a predecessor from a block
void zir_block_remove_predecessor(ZIRBlock *block, ZIRBlock *pred)
{
    if (!block || !pred)
        return;

    for (int i = 0; i < block->pred_count; i++)
    {
        if (block->predecessors[i] == pred)
        {
            // Shift remaining predecessors down
            for (int j = i; j < block->pred_count - 1; j++)
            {
                block->predecessors[j] = block->predecessors[j + 1];
            }
            block->pred_count--;

            // Remove this block as a successor from pred if present
            if (zir_block_has_successor(pred, block))
            {
                zir_block_remove_successor(pred, block);
            }
            return;
        }
    }
}

// Remove a successor from a block
void zir_block_remove_successor(ZIRBlock *block, ZIRBlock *succ)
{
    if (!block || !succ)
        return;

    for (int i = 0; i < block->succ_count; i++)
    {
        if (block->successors[i] == succ)
        {
            // Shift remaining successors down
            for (int j = i; j < block->succ_count - 1; j++)
            {
                block->successors[j] = block->successors[j + 1];
            }
            block->succ_count--;

            // Remove this block as a predecessor from succ if present
            if (zir_block_has_predecessor(succ, block))
            {
                zir_block_remove_predecessor(succ, block);
            }
            return;
        }
    }
}

// Check if a block has a specific predecessor
bool zir_block_has_predecessor(ZIRBlock *block, ZIRBlock *pred)
{
    if (!block || !pred)
        return false;

    for (int i = 0; i < block->pred_count; i++)
    {
        if (block->predecessors[i] == pred)
        {
            return true;
        }
    }
    return false;
}

// Check if a block has a specific successor
bool zir_block_has_successor(ZIRBlock *block, ZIRBlock *succ)
{
    if (!block || !succ)
        return false;

    for (int i = 0; i < block->succ_count; i++)
    {
        if (block->successors[i] == succ)
        {
            return true;
        }
    }
    return false;
}