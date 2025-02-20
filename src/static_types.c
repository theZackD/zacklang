#include "../include/static_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <stdbool.h>

// Create a new type instance
Type *create_type(BasicTypeKind kind)
{
    Type *type = malloc(sizeof(Type));
    if (!type)
    {
        fprintf(stderr, "Failed to allocate memory for type\n");
        exit(1);
    }
    type->kind = kind;
    type->is_const = false;
    type->is_comptime = false;
    return type;
}

// Create a const type instance
Type *create_const_type(BasicTypeKind kind)
{
    Type *type = create_type(kind);
    type->is_const = true;
    return type;
}

// Create a comptime type instance
Type *create_comptime_type(BasicTypeKind kind)
{
    Type *type = create_type(kind);
    type->is_comptime = true;
    return type;
}

// Convert a type string to a Type instance
Type *type_from_string(const char *type_str)
{
    if (!type_str)
        return create_type(TYPE_UNKNOWN);

    if (strcmp(type_str, "i32") == 0)
        return create_type(TYPE_I32);
    if (strcmp(type_str, "i64") == 0)
        return create_type(TYPE_I64);
    if (strcmp(type_str, "f32") == 0)
        return create_type(TYPE_F32);
    if (strcmp(type_str, "f64") == 0)
        return create_type(TYPE_F64);
    if (strcmp(type_str, "bool") == 0)
        return create_type(TYPE_BOOL);
    if (strcmp(type_str, "char") == 0)
        return create_type(TYPE_CHAR);
    if (strcmp(type_str, "string") == 0)
        return create_type(TYPE_STRING);
    if (strcmp(type_str, "void") == 0)
        return create_type(TYPE_VOID);

    return create_type(TYPE_UNKNOWN);
}

// Convert a Type to its string representation
const char *type_to_string(const Type *type)
{
    if (!type)
        return "unknown";

    static char buffer[256];
    const char *base_type;

    switch (type->kind)
    {
    case TYPE_I32:
        base_type = "i32";
        break;
    case TYPE_I64:
        base_type = "i64";
        break;
    case TYPE_F32:
        base_type = "f32";
        break;
    case TYPE_F64:
        base_type = "f64";
        break;
    case TYPE_BOOL:
        base_type = "bool";
        break;
    case TYPE_CHAR:
        base_type = "char";
        break;
    case TYPE_STRING:
        base_type = "string";
        break;
    case TYPE_VOID:
        base_type = "void";
        break;
    case TYPE_STRUCT:
        snprintf(buffer, sizeof(buffer), "struct %s",
                 type->info.struct_info->name);
        return buffer;
    case TYPE_UNKNOWN:
        return "unknown";
    case TYPE_ERROR:
        return "error";
    default:
        return "invalid";
    }

    snprintf(buffer, sizeof(buffer), "%s%s%s",
             type->is_comptime ? "comptime " : "",
             type->is_const ? "const " : "",
             base_type);
    return buffer;
}

// Check if two types are exactly the same
bool types_are_equal(const Type *t1, const Type *t2)
{
    if (!t1 || !t2)
        return false;
    return t1->kind == t2->kind &&
           t1->is_const == t2->is_const &&
           t1->is_comptime == t2->is_comptime;
}

// Check if a value of source type can be safely used where target type is expected
bool type_is_safe_for(const Type *source, const Type *target)
{
    if (!source || !target)
        return false;

    // Types must be of the same kind (no implicit conversions)
    if (source->kind != target->kind)
        return false;

    // Can't assign to a const target
    if (target->is_const)
        return false;

    // Can't use runtime value where comptime is expected
    if (target->is_comptime && !source->is_comptime)
        return false;

    return true;
}

// Get the result type of a binary operation
Type *get_binary_op_type(const char *op, const Type *left, const Type *right)
{
    if (!op || !left || !right)
        return create_type(TYPE_ERROR);

    // Result is comptime if both operands are comptime
    bool is_comptime = left->is_comptime && right->is_comptime;

    // Arithmetic operators
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 ||
        strcmp(op, "*") == 0 || strcmp(op, "/") == 0 ||
        strcmp(op, "%") == 0 || strcmp(op, "**") == 0)
    {
        // String concatenation
        if (strcmp(op, "+") == 0 && (left->kind == TYPE_STRING || right->kind == TYPE_STRING))
        {
            Type *result = create_type(TYPE_STRING);
            result->is_comptime = is_comptime;
            return result;
        }

        // Numeric operations
        if (is_numeric_type(left) && is_numeric_type(right))
        {
            // If either operand is f64, result is f64
            if (left->kind == TYPE_F64 || right->kind == TYPE_F64)
            {
                Type *result = create_type(TYPE_F64);
                result->is_comptime = is_comptime;
                return result;
            }
            // If either operand is f32, result is f32
            if (left->kind == TYPE_F32 || right->kind == TYPE_F32)
            {
                Type *result = create_type(TYPE_F32);
                result->is_comptime = is_comptime;
                return result;
            }
            // If either operand is i64, result is i64
            if (left->kind == TYPE_I64 || right->kind == TYPE_I64)
            {
                Type *result = create_type(TYPE_I64);
                result->is_comptime = is_comptime;
                return result;
            }
            // Otherwise result is i32
            Type *result = create_type(TYPE_I32);
            result->is_comptime = is_comptime;
            return result;
        }
    }

    // Comparison operators
    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
        strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0)
    {
        if (left->kind == right->kind)
        {
            Type *result = create_type(TYPE_BOOL);
            result->is_comptime = is_comptime;
            return result;
        }
        // Allow comparison between different numeric types
        if (is_numeric_type(left) && is_numeric_type(right))
        {
            Type *result = create_type(TYPE_BOOL);
            result->is_comptime = is_comptime;
            return result;
        }
    }

    // Logical operators
    if (strcmp(op, "and") == 0 || strcmp(op, "or") == 0 || strcmp(op, "xor") == 0)
    {
        if (left->kind == TYPE_BOOL && right->kind == TYPE_BOOL)
        {
            Type *result = create_type(TYPE_BOOL);
            result->is_comptime = is_comptime;
            return result;
        }
    }

    return create_type(TYPE_ERROR);
}

// Get the result type of a unary operation
Type *get_unary_op_type(const char *op, const Type *operand)
{
    if (!op || !operand)
        return create_type(TYPE_ERROR);

    Type *result = NULL;

    if (strcmp(op, "-") == 0 || strcmp(op, "+") == 0)
    {
        if (is_numeric_type(operand))
        {
            result = create_type(operand->kind);
        }
        else
        {
            return create_type(TYPE_ERROR);
        }
    }
    else if (strcmp(op, "not") == 0)
    {
        if (operand->kind == TYPE_BOOL)
        {
            result = create_type(TYPE_BOOL);
        }
        else
        {
            return create_type(TYPE_ERROR);
        }
    }
    else
    {
        return create_type(TYPE_ERROR);
    }

    if (result)
    {
        result->is_comptime = operand->is_comptime;
    }
    return result;
}

// Check if a type can be used in a condition
bool type_is_condition_compatible(const Type *type)
{
    if (!type)
        return false;
    return type->kind == TYPE_BOOL;
}

// Free a type instance
void free_type(Type *type)
{
    if (!type)
        return;

    if (type->kind == TYPE_STRUCT)
    {
        for (int i = 0; i < type->info.struct_info->field_count; i++)
        {
            free(type->info.struct_info->fields[i].name);
            free_type(type->info.struct_info->fields[i].type);
        }
        free(type->info.struct_info->fields);
        free(type->info.struct_info->name);
        free(type->info.struct_info);
    }

    free(type);
}

// Get the default value string for a type
const char *get_type_default_value(const Type *type)
{
    if (!type)
        return "unknown";

    switch (type->kind)
    {
    case TYPE_I32:
    case TYPE_I64:
        return "0";
    case TYPE_F32:
    case TYPE_F64:
        return "0.0";
    case TYPE_BOOL:
        return "false";
    case TYPE_CHAR:
        return "'\0'";
    case TYPE_STRING:
        return "\"\"";
    case TYPE_VOID:
        return "";
    default:
        return "unknown";
    }
}

// Check if a literal string value is compatible with a type
bool is_literal_compatible_with_type(const char *literal_value, const Type *type)
{
    if (!literal_value || !type)
        return false;

    switch (type->kind)
    {
    case TYPE_I32:
    case TYPE_I64:
        return value_fits_in_type(literal_value, type);
    case TYPE_F32:
    case TYPE_F64:
        return value_fits_in_type(literal_value, type);
    case TYPE_BOOL:
        return strcmp(literal_value, "true") == 0 ||
               strcmp(literal_value, "false") == 0;
    case TYPE_CHAR:
        return strlen(literal_value) == 1 ||
               (strlen(literal_value) == 2 && literal_value[0] == '\\');
    case TYPE_STRING:
        return literal_value[0] == '"' &&
               literal_value[strlen(literal_value) - 1] == '"';
    default:
        return false;
    }
}

// Get the type of a literal value
Type *get_literal_type(const char *literal_value)
{
    if (!literal_value)
        return create_type(TYPE_ERROR);

    Type *type = NULL;

    // All literals are comptime by default
    if (literal_value[0] == '"')
    {
        type = create_type(TYPE_STRING);
    }
    else if (strcmp(literal_value, "true") == 0 || strcmp(literal_value, "false") == 0)
    {
        type = create_type(TYPE_BOOL);
    }
    else if (literal_value[0] == '\'')
    {
        type = create_type(TYPE_CHAR);
    }
    else if (strchr(literal_value, '.') || strchr(literal_value, 'e') || strchr(literal_value, 'E'))
    {
        type = create_type(TYPE_F64);
    }
    else
    {
        // Check for integer
        bool is_integer = true;
        const char *c = literal_value;
        if (*c == '-')
            c++;
        while (*c)
        {
            if (!isdigit(*c))
            {
                is_integer = false;
                break;
            }
            c++;
        }
        if (is_integer)
        {
            type = create_type(TYPE_I32);
        }
        else
        {
            return create_type(TYPE_ERROR);
        }
    }

    if (type)
    {
        type->is_comptime = true; // All literals are comptime
    }
    return type;
}

// Check if a type can be evaluated at compile time
bool is_comptime_evaluable(const Type *type)
{
    return type && type->is_comptime;
}

// Get the explicit conversion function name between types
const char *get_type_conversion_fn(const Type *from, const Type *to)
{
    if (!from || !to)
        return NULL;

    static char buffer[32];

    // Same type, no conversion needed
    if (types_are_equal(from, to))
        return NULL;

    // Generate conversion function name
    snprintf(buffer, sizeof(buffer), "as_%s", type_to_string(to));
    return buffer;
}

// Check if a type is numeric
bool is_numeric_type(const Type *type)
{
    return type && (is_integer_type(type) || is_float_type(type));
}

// Check if a type is an integer type
bool is_integer_type(const Type *type)
{
    return type && (type->kind == TYPE_I32 || type->kind == TYPE_I64);
}

// Check if a type is a floating point type
bool is_float_type(const Type *type)
{
    return type && (type->kind == TYPE_F32 || type->kind == TYPE_F64);
}

// Check if a value would fit in the target numeric type
bool value_fits_in_type(const char *value, const Type *type)
{
    if (!value)
        return false;

    switch (type->kind)
    {
    case TYPE_I32:
    {
        bool is_integer = true;
        const char *c = value;

        // Skip leading whitespace and sign
        while (isspace(*c))
            c++;
        if (*c == '+' || *c == '-')
            c++;

        // Check if all remaining characters are digits
        while (*c)
        {
            if (*c == '.')
            {
                is_integer = false;
                break;
            }
            if (!isdigit(*c))
                return false;
            c++;
        }

        if (is_integer)
        {
            long val = strtol(value, NULL, 10);
            return val >= INT_MIN && val <= INT_MAX;
        }
        return false;
    }

    case TYPE_I64:
    {
        bool is_integer = true;
        const char *c = value;

        while (isspace(*c))
            c++;
        if (*c == '+' || *c == '-')
            c++;

        while (*c)
        {
            if (*c == '.')
            {
                is_integer = false;
                break;
            }
            if (!isdigit(*c))
                return false;
            c++;
        }

        if (is_integer)
        {
            long long val = strtoll(value, NULL, 10);
            return val >= LLONG_MIN && val <= LLONG_MAX;
        }
        return false;
    }

    case TYPE_F32:
    {
        char *endptr;
        float val = strtof(value, &endptr);

        // Check if conversion was successful and entire string was used
        if (*endptr != '\0' && !isspace(*endptr))
            return false;

        return val >= -FLT_MAX && val <= FLT_MAX;
    }

    case TYPE_F64:
    {
        char *endptr;
        double val = strtod(value, &endptr);

        // Check if conversion was successful and entire string was used
        if (*endptr != '\0' && !isspace(*endptr))
            return false;

        return val >= -DBL_MAX && val <= DBL_MAX;
    }

    case TYPE_BOOL:
        return strcmp(value, "true") == 0 ||
               strcmp(value, "false") == 0;

    case TYPE_STRING:
        return true; // All literals can be converted to strings

    case TYPE_VOID:
    case TYPE_ERROR:
    case TYPE_UNKNOWN:
    case TYPE_CHAR:
        return false;
    }

    return false;
}

// Create a new struct type
Type *create_struct_type(const char *name, StructField *fields, int field_count)
{
    Type *type = malloc(sizeof(Type));
    if (!type)
    {
        fprintf(stderr, "Failed to allocate memory for struct type\n");
        exit(1);
    }

    type->kind = TYPE_STRUCT;
    type->is_const = false;
    type->is_comptime = false;

    type->info.struct_info = malloc(sizeof(StructType));
    if (!type->info.struct_info)
    {
        free(type);
        fprintf(stderr, "Failed to allocate memory for struct info\n");
        exit(1);
    }

    type->info.struct_info->name = strdup(name);
    type->info.struct_info->field_count = field_count;

    // Allocate and copy fields
    type->info.struct_info->fields = malloc(field_count * sizeof(StructField));
    if (!type->info.struct_info->fields)
    {
        free(type->info.struct_info->name);
        free(type->info.struct_info);
        free(type);
        fprintf(stderr, "Failed to allocate memory for struct fields\n");
        exit(1);
    }

    memcpy(type->info.struct_info->fields, fields, field_count * sizeof(StructField));

    return type;
}

// Look up a field in a struct type
StructField *lookup_struct_field(const Type *struct_type, const char *field_name)
{
    if (struct_type->kind != TYPE_STRUCT)
    {
        return NULL;
    }

    for (int i = 0; i < struct_type->info.struct_info->field_count; i++)
    {
        if (strcmp(struct_type->info.struct_info->fields[i].name, field_name) == 0)
        {
            return &struct_type->info.struct_info->fields[i];
        }
    }

    return NULL;
}

// Create a struct field
StructField create_struct_field(const char *name, Type *type)
{
    StructField field;
    field.name = strdup(name);
    field.type = type;
    return field;
}
