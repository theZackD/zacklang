// include/zir_c_api.h
#ifndef ZIR_C_API_H
#define ZIR_C_API_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Opaque builder type
    struct ZIRBuilder_t;
    typedef struct ZIRBuilder_t ZIRBuilder;

    // Opaque handle types
    typedef void *zir_type_handle;
    typedef void *zir_value_handle;
    typedef void *zir_block_handle;
    typedef void *zir_function_handle;
    typedef void *zir_instruction_handle;

    // Opaque types
    typedef void *zir_module_handle;

    // Opaque types for value numbering
    typedef struct ZIRValueMap ZIRValueMap;
    typedef struct ZIRRedundantPairs ZIRRedundantPairs;
    typedef void *ZIRBasicBlock;
    typedef void *ZIRFunction;

    // Basic builder lifecycle
    ZIRBuilder *zir_create_builder(void);
    void zir_destroy_builder(ZIRBuilder *builder);
    const char *zir_get_version(ZIRBuilder *builder);

    // Type creation functions
    zir_type_handle zir_create_i32_type(void);
    zir_type_handle zir_create_i64_type(void);
    zir_type_handle zir_create_f32_type(void);
    zir_type_handle zir_create_f64_type(void);
    zir_type_handle zir_create_bool_type(void);
    zir_type_handle zir_create_string_type(void);

    // Literal creation functions
    zir_value_handle zir_create_integer_literal(zir_type_handle type, int64_t value);
    zir_value_handle zir_create_float_literal(zir_type_handle type, double value);
    zir_value_handle zir_create_bool_literal(zir_type_handle type, bool value);
    zir_value_handle zir_create_string_literal(zir_type_handle type, const char *value);

    // Type checking functions
    bool zir_is_integer_type(zir_type_handle type);
    bool zir_is_float_type(zir_type_handle type);
    bool zir_is_bool_type(zir_type_handle type);
    bool zir_is_string_type(zir_type_handle type);

    // Value access functions
    int64_t zir_get_integer_value(zir_value_handle value);
    double zir_get_float_value(zir_value_handle value);
    bool zir_get_bool_value(zir_value_handle value);
    const char *zir_get_string_value(zir_value_handle value);

    // Cleanup functions
    void zir_destroy_type(zir_type_handle type);
    void zir_destroy_value(zir_value_handle handle);

    // Function management
    zir_function_handle zir_create_function(const char *name);
    void zir_destroy_function(zir_function_handle handle);
    const char *zir_get_function_name(zir_function_handle handle);
    void zir_set_function_name(zir_function_handle handle, const char *name);

    // Block management
    zir_block_handle zir_create_basic_block(const char *name);
    void zir_destroy_basic_block(zir_block_handle handle);
    const char *zir_get_block_name(zir_block_handle handle);
    void zir_set_block_name(zir_block_handle handle, const char *name);
    uint64_t zir_get_block_id(zir_block_handle handle);

    // Function-block relationships
    bool zir_function_add_block(zir_function_handle func_handle, zir_block_handle block_handle);
    void zir_function_remove_block(zir_function_handle func_handle, zir_block_handle block_handle);
    size_t zir_function_get_block_count(zir_function_handle handle);
    zir_block_handle zir_function_get_block(zir_function_handle handle, size_t index);
    void zir_set_block_parent(zir_block_handle block, zir_function_handle function);
    zir_function_handle zir_get_block_parent(zir_block_handle block);

    // Dead block analysis
    size_t zir_function_remove_dead_blocks(zir_function_handle handle);
    bool zir_block_is_dead(zir_block_handle block, zir_function_handle func_handle);

    // Block linking
    void zir_block_add_predecessor(zir_block_handle block, zir_block_handle pred);
    void zir_block_add_successor(zir_block_handle block, zir_block_handle succ);
    void zir_block_remove_predecessor(zir_block_handle block, zir_block_handle pred);
    void zir_block_remove_successor(zir_block_handle block, zir_block_handle succ);
    size_t zir_block_get_predecessor_count(zir_block_handle block);
    size_t zir_block_get_successor_count(zir_block_handle block);
    bool zir_block_has_predecessor(zir_block_handle block, zir_block_handle pred);
    bool zir_block_has_successor(zir_block_handle block, zir_block_handle succ);

    // Block merging
    bool zir_block_is_mergeable_with(zir_block_handle block, zir_block_handle other);
    bool zir_block_is_safe_merge_with(zir_block_handle block, zir_block_handle other);
    zir_block_handle zir_block_merge_with(zir_block_handle block, zir_block_handle other);
    zir_block_handle zir_block_find_mergeable_successor(zir_block_handle block);

    // Jump threading analysis
    bool zir_block_is_jump_threadable(zir_block_handle block);
    bool zir_block_can_thread_jump_through(zir_block_handle block);
    zir_block_handle zir_block_get_jump_target(zir_block_handle block);
    bool zir_block_is_jump_threading_safe(zir_block_handle block, zir_block_handle from, zir_block_handle to);

    // Jump threading transformation
    bool zir_block_perform_jump_threading(zir_block_handle block, zir_block_handle from, zir_block_handle to);

    // Critical edge detection and validation
    bool zir_block_has_critical_edges(zir_block_handle block);
    bool zir_block_is_critical_edge_to_successor(zir_block_handle block, zir_block_handle succ);
    bool zir_block_is_split_safe(zir_block_handle block, zir_block_handle succ);
    size_t zir_block_find_critical_edges(zir_block_handle block,
                                         zir_block_handle *source_blocks,
                                         zir_block_handle *target_blocks,
                                         size_t max_edges);

    // Critical edge splitting
    zir_block_handle zir_block_split_critical_edge(zir_block_handle block, zir_block_handle succ);
    size_t zir_block_split_all_critical_edges(zir_block_handle block);

    // Jump threading opportunities
    size_t zir_block_find_jump_threading_opportunities(zir_block_handle block,
                                                       zir_block_handle *from_blocks,
                                                       zir_block_handle *to_blocks,
                                                       size_t max_opportunities);

    // Value management
    zir_value_handle zir_create_int32_value(int32_t value);
    zir_value_handle zir_create_int64_value(int64_t value);
    zir_value_handle zir_create_float_value(float value);
    zir_value_handle zir_create_double_value(double value);
    zir_value_handle zir_create_bool_value(bool value);

    // Instruction management
    void zir_destroy_instruction(zir_instruction_handle handle);
    const char *zir_get_instruction_name(zir_instruction_handle handle);
    zir_value_handle zir_instruction_get_left_operand(zir_instruction_handle handle);
    zir_value_handle zir_instruction_get_right_operand(zir_instruction_handle handle);
    zir_value_handle zir_instruction_get_operand(zir_instruction_handle handle);
    void zir_block_add_instruction(zir_block_handle block, zir_instruction_handle instruction);
    void zir_block_remove_instruction(zir_block_handle block, size_t index);
    zir_instruction_handle zir_block_get_instruction(zir_block_handle block, size_t index);
    size_t zir_block_get_instruction_count(zir_block_handle block);

    // Arithmetic instructions
    zir_instruction_handle zir_create_add_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_sub_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_mul_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_div_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_mod_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_pow_instruction(zir_value_handle left, zir_value_handle right);

    // Comparison instructions
    zir_instruction_handle zir_create_eq_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_ne_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_lt_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_le_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_gt_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_ge_instruction(zir_value_handle left, zir_value_handle right);

    // Logical instructions
    zir_instruction_handle zir_create_and_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_or_instruction(zir_value_handle left, zir_value_handle right);
    zir_instruction_handle zir_create_not_instruction(zir_value_handle operand);

    // Control flow instructions
    zir_instruction_handle zir_create_jump_instruction(zir_block_handle target);
    zir_instruction_handle zir_create_branch_instruction(zir_value_handle condition,
                                                         zir_block_handle true_target,
                                                         zir_block_handle false_target);
    zir_instruction_handle zir_create_return_instruction(zir_value_handle value);
    zir_instruction_handle zir_create_void_return_instruction(void);

    // Control flow instruction accessors
    zir_value_handle zir_branch_get_condition(zir_instruction_handle branch);
    zir_block_handle zir_branch_get_true_block(zir_instruction_handle branch);
    zir_block_handle zir_branch_get_false_block(zir_instruction_handle branch);
    zir_block_handle zir_jump_get_target(zir_instruction_handle jump);
    zir_value_handle zir_return_get_value(zir_instruction_handle ret);
    bool zir_return_is_void(zir_instruction_handle ret);

    // Value Numbering API

    /**
     * Checks if a basic block has redundant computations
     *
     * @param block The ZIRBasicBlock to check
     * @return 1 if there are redundant computations, 0 otherwise
     */
    int ZIRBasicBlock_hasRedundantComputations(ZIRBasicBlock *block);

    /**
     * Performs local value numbering on a basic block
     *
     * @param block The ZIRBasicBlock to process
     * @return A ZIRValueMap containing the value numbering results
     */
    ZIRValueMap *ZIRBasicBlock_performLocalValueNumbering(ZIRBasicBlock *block);

    /**
     * Finds redundant computations in a basic block
     *
     * @param block The ZIRBasicBlock to analyze
     * @return A ZIRRedundantPairs containing pairs of redundant instructions
     */
    ZIRRedundantPairs *ZIRBasicBlock_findRedundantComputations(ZIRBasicBlock *block);

    /**
     * Checks if a function has redundant computations across basic blocks
     *
     * @param function The ZIRFunction to check
     * @return 1 if there are redundant computations, 0 otherwise
     */
    int ZIRFunction_hasGlobalRedundantComputations(ZIRFunction *function);

    /**
     * Performs global value numbering on a function
     *
     * @param function The ZIRFunction to process
     * @return A ZIRValueMap containing the global value numbering results
     */
    ZIRValueMap *ZIRFunction_performGlobalValueNumbering(ZIRFunction *function);

    /**
     * Finds redundant computations across basic blocks in a function
     *
     * @param function The ZIRFunction to analyze
     * @return A ZIRRedundantPairs containing pairs of redundant instructions
     */
    ZIRRedundantPairs *ZIRFunction_findGlobalRedundantComputations(ZIRFunction *function);

    // Clean up value numbering resources

    /**
     * Releases the memory allocated for a ZIRValueMap
     *
     * @param value_map The ZIRValueMap to release
     */
    void ZIRValueMap_destroy(ZIRValueMap *value_map);

    /**
     * Releases the memory allocated for a ZIRRedundantPairs
     *
     * @param redundant_pairs The ZIRRedundantPairs to release
     */
    void ZIRRedundantPairs_destroy(ZIRRedundantPairs *redundant_pairs);

#ifdef __cplusplus
}
#endif

#endif // ZIR_C_API_H