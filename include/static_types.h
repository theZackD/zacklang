#ifndef STATIC_TYPES_H
#define STATIC_TYPES_H

#include <stdbool.h>

// Basic type enumeration.
typedef enum
{
    TYPE_I32,
    TYPE_I64,
    TYPE_F32,
    TYPE_F64,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_STRUCT, // For struct types.
    TYPE_UNKNOWN,
    TYPE_ERROR
} BasicTypeKind;

// Structure to represent a struct field.
typedef struct StructField
{
    char *name;
    struct Type *type;
} StructField;

// Structure to represent a struct type.
typedef struct StructType
{
    char *name;
    StructField *fields;
    int field_count;
} StructType;

// Type structure (can be extended for more complex types later).
typedef struct Type
{
    BasicTypeKind kind;
    bool is_const;    // Whether the type is immutable (const).
    bool is_comptime; // Whether the value must be known at compile time.
    union
    {
        StructType *struct_info; // Only used if kind == TYPE_STRUCT.
    } info;
} Type;

// Function prototypes:

// Create a new type instance.
Type *create_type(BasicTypeKind kind);

// Create a const type instance.
Type *create_const_type(BasicTypeKind kind);

// Create a comptime type instance.
Type *create_comptime_type(BasicTypeKind kind);

// Convert a type string to a Type instance.
Type *type_from_string(const char *type_str);

// Convert a Type to its string representation.
const char *type_to_string(const Type *type);

// Check if two types are exactly the same (including const and comptime flags).
bool types_are_equal(const Type *t1, const Type *t2);

// Check if a value of the source type can be safely used where the target type is expected.
bool type_is_safe_for(const Type *source, const Type *target);

// Get the result type of a binary operation.
Type *get_binary_op_type(const char *op, const Type *left, const Type *right);

// Get the result type of a unary operation.
Type *get_unary_op_type(const char *op, const Type *operand);

// Check if a type can be used in a condition (if, while, etc.).
bool type_is_condition_compatible(const Type *type);

// Free a type instance.
void free_type(Type *type);

// Get the default value string for a type.
const char *get_type_default_value(const Type *type);

// Check if a literal string value is compatible with a type.
bool is_literal_compatible_with_type(const char *literal_value, const Type *type);

// Get the type of a literal value.
Type *get_literal_type(const char *literal_value);

// Check if a type can be evaluated at compile time.
bool is_comptime_evaluable(const Type *type);

// Get the explicit conversion function name between types (or NULL if not applicable).
const char *get_type_conversion_fn(const Type *from, const Type *to);

// Check if a type is numeric (integer or floating point).
bool is_numeric_type(const Type *type);

// Check if a type is an integer type.
bool is_integer_type(const Type *type);

// Check if a type is a floating point type.
bool is_float_type(const Type *type);

// Check if a value would fit in the target numeric type.
bool value_fits_in_type(const char *value, const Type *type);

// Create a new struct type.
Type *create_struct_type(const char *name, StructField *fields, int field_count);

// Look up a field in a struct type.
StructField *lookup_struct_field(const Type *struct_type, const char *field_name);

// Create a struct field.
StructField create_struct_field(const char *name, Type *type);

#endif // STATIC_TYPES_H
