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

    // Opaque type declarations
    struct ZIRBuilder_t;
    typedef struct ZIRBuilder_t ZIRBuilder;

    struct ZIRType_t;
    typedef struct ZIRType_t *ZIRTypeHandle;

    struct ZIRValue_t;
    typedef struct ZIRValue_t *ZIRValueHandle;

    struct ZIRBasicBlock_t;
    typedef struct ZIRBasicBlock_t *ZIRBasicBlockHandle;

    struct ZIRFunction_t;
    typedef struct ZIRFunction_t *ZIRFunctionHandle;

    // Basic builder lifecycle
    ZIRBuilder *zir_create_builder(void);
    void zir_destroy_builder(ZIRBuilder *builder);

    // Test function to verify everything works
    const char *zir_get_version(ZIRBuilder *builder);

    // Type creation functions
    ZIRTypeHandle zir_create_i32_type(void);
    ZIRTypeHandle zir_create_i64_type(void);
    ZIRTypeHandle zir_create_f32_type(void);
    ZIRTypeHandle zir_create_f64_type(void);
    ZIRTypeHandle zir_create_bool_type(void);
    ZIRTypeHandle zir_create_string_type(void);

    // Literal creation functions
    ZIRValueHandle zir_create_integer_literal(ZIRTypeHandle type, int64_t value);
    ZIRValueHandle zir_create_float_literal(ZIRTypeHandle type, double value);
    ZIRValueHandle zir_create_bool_literal(ZIRTypeHandle type, bool value);
    ZIRValueHandle zir_create_string_literal(ZIRTypeHandle type, const char *value);

    // Type checking functions
    bool zir_is_integer_type(ZIRTypeHandle type);
    bool zir_is_float_type(ZIRTypeHandle type);
    bool zir_is_bool_type(ZIRTypeHandle type);
    bool zir_is_string_type(ZIRTypeHandle type);

    // Value access functions
    int64_t zir_get_integer_value(ZIRValueHandle value);
    double zir_get_float_value(ZIRValueHandle value);
    bool zir_get_bool_value(ZIRValueHandle value);
    const char *zir_get_string_value(ZIRValueHandle value);

    // Cleanup functions
    void zir_destroy_type(ZIRTypeHandle type);
    void zir_destroy_value(ZIRValueHandle value);

    // Basic block functions
    ZIRBasicBlockHandle zir_create_basic_block(const char *name);
    void zir_destroy_basic_block(ZIRBasicBlockHandle block);
    const char *zir_get_block_name(ZIRBasicBlockHandle block);
    void zir_set_block_name(ZIRBasicBlockHandle block, const char *name);
    uint64_t zir_get_block_id(ZIRBasicBlockHandle block);

    // Function management
    ZIRFunctionHandle zir_create_function(const char *name);
    void zir_destroy_function(ZIRFunctionHandle function);
    const char *zir_get_function_name(ZIRFunctionHandle function);
    void zir_set_function_name(ZIRFunctionHandle function, const char *name);

    // Function-Block relationship
    void zir_set_block_parent(ZIRBasicBlockHandle block, ZIRFunctionHandle function);
    ZIRFunctionHandle zir_get_block_parent(ZIRBasicBlockHandle block);

    // Block list management
    void zir_function_add_block(ZIRFunctionHandle function, ZIRBasicBlockHandle block);
    void zir_function_remove_block(ZIRFunctionHandle function, ZIRBasicBlockHandle block);
    size_t zir_function_get_block_count(ZIRFunctionHandle function);
    ZIRBasicBlockHandle zir_function_get_block(ZIRFunctionHandle function, size_t index);

#ifdef __cplusplus
}
#endif

#endif // ZIR_C_API_H