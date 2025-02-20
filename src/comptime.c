#include "../include/comptime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Create a new comptime value
ComptimeValue *create_comptime_value(Type *type)
{
    ComptimeValue *value = malloc(sizeof(ComptimeValue));
    if (!value)
    {
        fprintf(stderr, "Failed to allocate comptime value\n");
        exit(1);
    }
    value->type = type;

    // Initialize union based on type
    if (type->kind == TYPE_STRUCT)
    {
        value->value.struct_val.type_name = strdup(type->info.struct_info->name);
        value->value.struct_val.field_count = type->info.struct_info->field_count;
        value->value.struct_val.field_values = calloc(type->info.struct_info->field_count,
                                                      sizeof(ComptimeValue *));
    }
    else
    {
        // Initialize to 0/false/NULL
        memset(&value->value, 0, sizeof(value->value));
    }

    return value;
}

// Free a comptime value
void free_comptime_value(ComptimeValue *value)
{
    if (!value)
        return;

    if (value->type->kind == TYPE_STRING && value->value.s_val)
    {
        free(value->value.s_val);
    }
    else if (value->type->kind == TYPE_STRUCT)
    {
        free(value->value.struct_val.type_name);
        for (int i = 0; i < value->value.struct_val.field_count; i++)
        {
            free_comptime_value(value->value.struct_val.field_values[i]);
        }
        free(value->value.struct_val.field_values);
    }
    else if (value->type->kind == TYPE_ARRAY)
    {
        for (int i = 0; i < value->value.array_val.length; i++)
        {
            free_comptime_value(value->value.array_val.elements[i]);
        }
        free(value->value.array_val.elements);
    }

    // Free the type
    if (value->type->kind == TYPE_ARRAY)
    {
        free_type(value->type->info.array_info->element_type);
        free(value->type->info.array_info);
    }
    free_type(value->type);
    free(value);
}

// Convert a comptime value to a string
char *comptime_value_to_string(ComptimeValue *value)
{
    if (!value)
        return strdup("null");

    char buffer[256];
    switch (value->type->kind)
    {
    case TYPE_I32:
    case TYPE_I64:
        snprintf(buffer, sizeof(buffer), "%lld", value->value.i_val);
        break;
    case TYPE_F32:
    case TYPE_F64:
        snprintf(buffer, sizeof(buffer), "%g", value->value.f_val);
        break;
    case TYPE_BOOL:
        snprintf(buffer, sizeof(buffer), "%s", value->value.b_val ? "true" : "false");
        break;
    case TYPE_STRING:
        return strdup(value->value.s_val ? value->value.s_val : "");
    case TYPE_STRUCT:
        snprintf(buffer, sizeof(buffer), "struct %s {...}", value->value.struct_val.type_name);
        break;
    default:
        snprintf(buffer, sizeof(buffer), "<unknown>");
    }
    return strdup(buffer);
}

// Check if an expression can be evaluated at compile time
bool is_comptime_expr(ASTNode *expr)
{
    if (!expr)
        return false;

    switch (expr->type)
    {
    case AST_LITERAL:
        return true;

    case AST_IDENTIFIER:
        // Only const variables can be comptime
        // TODO: Look up in symbol table and check if const
        return false;

    case AST_BINARY_EXPR:
        return is_comptime_expr(expr->data.binary_expr.left) &&
               is_comptime_expr(expr->data.binary_expr.right);

    case AST_UNARY_EXPR:
        return is_comptime_expr(expr->data.unary_expr.operand);

    case AST_FUNC_CALL:
        // Only comptime functions can be evaluated at compile time
        // TODO: Look up function and check if comptime
        return false;

    default:
        return false;
    }
}

// Convert a literal to a comptime value
ComptimeValue *literal_to_comptime_value(const char *literal_value, Type *type)
{
    printf("DEBUG: Converting literal '%s' of type %s to comptime value\n",
           literal_value, type_to_string(type));

    ComptimeValue *value = create_comptime_value(type);
    if (!value)
    {
        printf("DEBUG: Failed to create comptime value\n");
        return NULL;
    }

    switch (type->kind)
    {
    case TYPE_I32:
    case TYPE_I64:
        printf("DEBUG: Converting to integer\n");
        value->value.i_val = strtol(literal_value, NULL, 10);
        break;

    case TYPE_F32:
    case TYPE_F64:
        printf("DEBUG: Converting to float\n");
        value->value.f_val = strtod(literal_value, NULL);
        break;

    case TYPE_BOOL:
        printf("DEBUG: Converting to boolean\n");
        value->value.b_val = strcmp(literal_value, "true") == 0;
        break;

    case TYPE_STRING:
        printf("DEBUG: Converting to string\n");
        // Remove quotes and handle escapes
        value->value.s_val = strdup(literal_value + 1);
        value->value.s_val[strlen(value->value.s_val) - 1] = '\0';
        break;

    default:
        printf("DEBUG: Unsupported literal type %d\n", type->kind);
        fprintf(stderr, "Unsupported literal type for comptime evaluation\n");
        free_comptime_value(value);
        return NULL;
    }

    printf("DEBUG: Successfully converted literal\n");
    return value;
}

// Evaluate a binary operation at compile time
ComptimeValue *evaluate_comptime_binary_op(const char *op, ComptimeValue *left, ComptimeValue *right)
{
    printf("DEBUG: Evaluating binary operation '%s'\n", op);

    if (!left || !right)
    {
        printf("DEBUG: Null operand in binary operation\n");
        return NULL;
    }

    printf("DEBUG: Left operand type: %s\n", type_to_string(left->type));
    printf("DEBUG: Right operand type: %s\n", type_to_string(right->type));

    // Handle logical operators first
    if (left->type->kind == TYPE_BOOL && right->type->kind == TYPE_BOOL)
    {
        printf("DEBUG: Both operands are boolean\n");
        ComptimeValue *result = create_comptime_value(create_type(TYPE_BOOL));

        if (strcmp(op, "and") == 0)
        {
            result->value.b_val = left->value.b_val && right->value.b_val;
            printf("DEBUG: Logical AND result: %s\n", result->value.b_val ? "true" : "false");
        }
        else if (strcmp(op, "or") == 0)
        {
            result->value.b_val = left->value.b_val || right->value.b_val;
            printf("DEBUG: Logical OR result: %s\n", result->value.b_val ? "true" : "false");
        }
        else if (strcmp(op, "==") == 0)
        {
            result->value.b_val = left->value.b_val == right->value.b_val;
            printf("DEBUG: Boolean equality result: %s\n", result->value.b_val ? "true" : "false");
        }
        else if (strcmp(op, "!=") == 0)
        {
            result->value.b_val = left->value.b_val != right->value.b_val;
            printf("DEBUG: Boolean inequality result: %s\n", result->value.b_val ? "true" : "false");
        }
        else
        {
            printf("DEBUG: Invalid boolean operation: %s\n", op);
            free_comptime_value(result);
            return NULL;
        }

        return result;
    }

    // Handle numeric operations
    if (is_numeric_type(left->type) && is_numeric_type(right->type))
    {
        printf("DEBUG: Both operands are numeric\n");

        // For comparison operators, create a boolean result
        if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
            strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 ||
            strcmp(op, ">") == 0 || strcmp(op, ">=") == 0)
        {
            ComptimeValue *result = create_comptime_value(create_type(TYPE_BOOL));

            // Convert to highest precision for comparison
            double l_val = left->type->kind >= TYPE_F32 ? left->value.f_val : (double)left->value.i_val;
            double r_val = right->type->kind >= TYPE_F32 ? right->value.f_val : (double)right->value.i_val;

            printf("DEBUG: Comparing numeric values: %g %s %g\n", l_val, op, r_val);

            if (strcmp(op, "==") == 0)
                result->value.b_val = l_val == r_val;
            else if (strcmp(op, "!=") == 0)
                result->value.b_val = l_val != r_val;
            else if (strcmp(op, "<") == 0)
                result->value.b_val = l_val < r_val;
            else if (strcmp(op, "<=") == 0)
                result->value.b_val = l_val <= r_val;
            else if (strcmp(op, ">") == 0)
                result->value.b_val = l_val > r_val;
            else if (strcmp(op, ">=") == 0)
                result->value.b_val = l_val >= r_val;

            printf("DEBUG: Comparison result: %s\n", result->value.b_val ? "true" : "false");
            return result;
        }

        // For arithmetic operators, get the result type
        Type *result_type = get_binary_op_type(op, left->type, right->type);
        if (!result_type)
        {
            printf("DEBUG: Failed to get result type for binary operation\n");
            return NULL;
        }

        printf("DEBUG: Result type will be: %s\n", type_to_string(result_type));
        ComptimeValue *result = create_comptime_value(result_type);

        // Convert to highest precision
        double l_val = left->type->kind >= TYPE_F32 ? left->value.f_val : (double)left->value.i_val;
        double r_val = right->type->kind >= TYPE_F32 ? right->value.f_val : (double)right->value.i_val;

        printf("DEBUG: Converted operands to: %g and %g\n", l_val, r_val);

        if (strcmp(op, "+") == 0)
        {
            result->value.f_val = l_val + r_val;
            printf("DEBUG: Addition result: %g\n", result->value.f_val);
        }
        else if (strcmp(op, "-") == 0)
        {
            result->value.f_val = l_val - r_val;
            printf("DEBUG: Subtraction result: %g\n", result->value.f_val);
        }
        else if (strcmp(op, "*") == 0)
        {
            result->value.f_val = l_val * r_val;
            printf("DEBUG: Multiplication result: %g\n", result->value.f_val);
        }
        else if (strcmp(op, "/") == 0)
        {
            if (r_val == 0)
            {
                printf("DEBUG: Division by zero error\n");
                free_comptime_value(result);
                return NULL;
            }
            result->value.f_val = l_val / r_val;
            printf("DEBUG: Division result: %g\n", result->value.f_val);
        }
        else if (strcmp(op, "**") == 0)
        {
            result->value.f_val = pow(l_val, r_val);
            printf("DEBUG: Power result: %g\n", result->value.f_val);
        }
        else if (strcmp(op, "%") == 0)
        {
            if (r_val == 0)
            {
                printf("DEBUG: Modulo by zero error\n");
                free_comptime_value(result);
                return NULL;
            }
            result->value.f_val = fmod(l_val, r_val);
            printf("DEBUG: Modulo result: %g\n", result->value.f_val);
        }

        // Convert back to integer if needed
        if (result_type->kind < TYPE_F32)
        {
            result->value.i_val = (int64_t)result->value.f_val;
            printf("DEBUG: Converted float result to integer: %lld\n", result->value.i_val);
        }

        return result;
    }

    // Handle string concatenation
    if (strcmp(op, "+") == 0 && left->type->kind == TYPE_STRING && right->type->kind == TYPE_STRING)
    {
        ComptimeValue *result = create_comptime_value(create_type(TYPE_STRING));
        size_t len = strlen(left->value.s_val) + strlen(right->value.s_val) + 1;
        result->value.s_val = malloc(len);
        snprintf(result->value.s_val, len, "%s%s", left->value.s_val, right->value.s_val);
        return result;
    }

    // Handle string comparison
    if ((strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
         strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 ||
         strcmp(op, ">") == 0 || strcmp(op, ">=") == 0) &&
        left->type->kind == TYPE_STRING && right->type->kind == TYPE_STRING)
    {
        ComptimeValue *result = create_comptime_value(create_type(TYPE_BOOL));
        int cmp = strcmp(left->value.s_val, right->value.s_val);

        if (strcmp(op, "==") == 0)
            result->value.b_val = cmp == 0;
        else if (strcmp(op, "!=") == 0)
            result->value.b_val = cmp != 0;
        else if (strcmp(op, "<") == 0)
            result->value.b_val = cmp < 0;
        else if (strcmp(op, "<=") == 0)
            result->value.b_val = cmp <= 0;
        else if (strcmp(op, ">") == 0)
            result->value.b_val = cmp > 0;
        else if (strcmp(op, ">=") == 0)
            result->value.b_val = cmp >= 0;

        return result;
    }

    printf("DEBUG: Unsupported binary operation '%s' between types %s and %s\n",
           op, type_to_string(left->type), type_to_string(right->type));
    return NULL;
}

// Evaluate a unary operation at compile time
ComptimeValue *evaluate_comptime_unary_op(const char *op, ComptimeValue *operand)
{
    if (!operand)
        return NULL;

    if (strcmp(op, "-") == 0 && is_numeric_type(operand->type))
    {
        ComptimeValue *result = create_comptime_value(operand->type);
        if (operand->type->kind >= TYPE_F32)
        {
            result->value.f_val = -operand->value.f_val;
        }
        else
        {
            result->value.i_val = -operand->value.i_val;
        }
        return result;
    }

    if (strcmp(op, "not") == 0 && operand->type->kind == TYPE_BOOL)
    {
        ComptimeValue *result = create_comptime_value(create_type(TYPE_BOOL));
        result->value.b_val = !operand->value.b_val;
        return result;
    }

    fprintf(stderr, "Unsupported unary operation for comptime evaluation\n");
    return NULL;
}

// Array operations
ComptimeValue *evaluate_array_literal(ASTNode **elements, int count, Type *element_type)
{
    printf("DEBUG: Evaluating array literal with %d elements\n", count);

    // Create array type
    Type *array_type = create_type(TYPE_ARRAY);
    array_type->info.array_info = malloc(sizeof(ArrayTypeInfo));
    array_type->info.array_info->element_type = create_type(element_type->kind);
    array_type->info.array_info->element_type->is_comptime = element_type->is_comptime;
    array_type->info.array_info->element_type->is_const = element_type->is_const;

    // Create array value
    ComptimeValue *array = create_comptime_value(array_type);
    array->value.array_val.length = count;
    array->value.array_val.elements = malloc(count * sizeof(ComptimeValue *));

    // Evaluate each element
    for (int i = 0; i < count; i++)
    {
        printf("DEBUG: Evaluating array element %d\n", i);
        array->value.array_val.elements[i] = evaluate_comptime_expr(elements[i]);
        if (!array->value.array_val.elements[i])
        {
            printf("DEBUG: Failed to evaluate array element %d\n", i);
            // Clean up previously allocated elements
            for (int j = 0; j < i; j++)
            {
                free_comptime_value(array->value.array_val.elements[j]);
            }
            free(array->value.array_val.elements);
            free(array_type->info.array_info->element_type);
            free(array_type->info.array_info);
            free(array_type);
            free(array);
            return NULL;
        }

        // Check element type matches
        if (array->value.array_val.elements[i]->type->kind != element_type->kind)
        {
            printf("DEBUG: Type mismatch in array element %d\n", i);
            // Clean up all allocated elements
            for (int j = 0; j <= i; j++)
            {
                free_comptime_value(array->value.array_val.elements[j]);
            }
            free(array->value.array_val.elements);
            free(array_type->info.array_info->element_type);
            free(array_type->info.array_info);
            free(array_type);
            free(array);
            return NULL;
        }
    }

    printf("DEBUG: Successfully created array value\n");
    return array;
}

ComptimeValue *evaluate_array_index(ComptimeValue *array, ComptimeValue *index)
{
    if (!array || !index)
    {
        printf("DEBUG: Null array or index in array indexing\n");
        return NULL;
    }

    if (array->type->kind != TYPE_ARRAY)
    {
        printf("DEBUG: Cannot index non-array type %s\n", type_to_string(array->type));
        return NULL;
    }

    if (index->type->kind != TYPE_I32 && index->type->kind != TYPE_I64)
    {
        printf("DEBUG: Array index must be integer, got %s\n", type_to_string(index->type));
        return NULL;
    }

    int64_t idx = index->value.i_val;
    if (idx < 0 || idx >= array->value.array_val.length)
    {
        printf("DEBUG: Array index %lld out of bounds [0, %d)\n",
               idx, array->value.array_val.length);
        return NULL;
    }

    // Create a new value with the element's type
    ComptimeValue *result = create_comptime_value(array->type->info.array_info->element_type);

    // Handle different element types
    switch (array->type->info.array_info->element_type->kind)
    {
    case TYPE_STRING:
        result->value.s_val = strdup(array->value.array_val.elements[idx]->value.s_val);
        break;
    case TYPE_ARRAY:
        // For arrays, we need to create a deep copy
        result->value.array_val.length = array->value.array_val.elements[idx]->value.array_val.length;
        result->value.array_val.elements = malloc(result->value.array_val.length * sizeof(ComptimeValue *));
        for (int i = 0; i < result->value.array_val.length; i++)
        {
            // Recursively copy array elements
            ComptimeValue *elem = array->value.array_val.elements[idx]->value.array_val.elements[i];
            result->value.array_val.elements[i] = create_comptime_value(elem->type);
            memcpy(&result->value.array_val.elements[i]->value, &elem->value, sizeof(elem->value));
        }
        break;
    case TYPE_STRUCT:
        // For structs, copy the struct info
        result->value.struct_val.type_name = strdup(array->value.array_val.elements[idx]->value.struct_val.type_name);
        result->value.struct_val.field_count = array->value.array_val.elements[idx]->value.struct_val.field_count;
        result->value.struct_val.field_values = malloc(result->value.struct_val.field_count * sizeof(ComptimeValue *));
        for (int i = 0; i < result->value.struct_val.field_count; i++)
        {
            result->value.struct_val.field_values[i] = create_comptime_value(array->value.array_val.elements[idx]->value.struct_val.field_values[i]->type);
            memcpy(&result->value.struct_val.field_values[i]->value,
                   &array->value.array_val.elements[idx]->value.struct_val.field_values[i]->value,
                   sizeof(result->value.struct_val.field_values[i]->value));
        }
        break;
    default:
        // For primitive types, a simple value copy is sufficient
        memcpy(&result->value, &array->value.array_val.elements[idx]->value, sizeof(result->value));
        break;
    }

    return result;
}

ComptimeValue *get_array_length(ComptimeValue *array)
{
    if (!array)
    {
        printf("DEBUG: Null array in length operation\n");
        return NULL;
    }

    if (array->type->kind != TYPE_ARRAY)
    {
        printf("DEBUG: Cannot get length of non-array type %s\n", type_to_string(array->type));
        return NULL;
    }

    ComptimeValue *result = create_comptime_value(create_type(TYPE_I32));
    result->value.i_val = array->value.array_val.length;
    return result;
}

// Main evaluation function with symbol table
ComptimeValue *evaluate_comptime_expr_with_symbols(ASTNode *expr, SymbolTable *symbols)
{
    if (!expr)
    {
        printf("DEBUG: evaluate_comptime_expr_with_symbols called with NULL expr\n");
        return NULL;
    }

    printf("DEBUG: Evaluating expression of type %d with symbols\n", expr->type);

    switch (expr->type)
    {
    case AST_LITERAL:
    {
        printf("DEBUG: Converting literal '%s' to comptime value\n", expr->data.literal.value);
        Type *type = get_literal_type(expr->data.literal.value);
        if (!type)
        {
            printf("DEBUG: Failed to get type for literal\n");
            return NULL;
        }
        printf("DEBUG: Got type %s for literal\n", type_to_string(type));
        return literal_to_comptime_value(expr->data.literal.value, type);
    }

    case AST_IDENTIFIER:
    {
        printf("DEBUG: Looking up identifier '%s' in symbol table\n", expr->data.identifier.name);
        Symbol *sym = lookup_symbol(symbols, expr->data.identifier.name);
        if (!sym)
        {
            printf("DEBUG: Symbol '%s' not found\n", expr->data.identifier.name);
            return NULL;
        }

        // Check if it's a const variable
        if (sym->node && sym->node->type == AST_VAR_DECL && sym->node->data.var_decl.is_const)
        {
            printf("DEBUG: Found const variable '%s', evaluating initializer\n", expr->data.identifier.name);
            return evaluate_comptime_expr_with_symbols(sym->node->data.var_decl.initializer, symbols);
        }

        printf("DEBUG: Symbol '%s' is not a const variable\n", expr->data.identifier.name);
        return NULL;
    }

    case AST_BINARY_EXPR:
    {
        printf("DEBUG: Evaluating binary expression with operator '%s'\n", expr->data.binary_expr.op);
        printf("DEBUG: Evaluating left operand...\n");
        ComptimeValue *left = evaluate_comptime_expr_with_symbols(expr->data.binary_expr.left, symbols);
        if (!left)
        {
            printf("DEBUG: Failed to evaluate left operand\n");
            return NULL;
        }
        printf("DEBUG: Successfully evaluated left operand\n");

        printf("DEBUG: Evaluating right operand...\n");
        ComptimeValue *right = evaluate_comptime_expr_with_symbols(expr->data.binary_expr.right, symbols);
        if (!right)
        {
            printf("DEBUG: Failed to evaluate right operand\n");
            free_comptime_value(left);
            return NULL;
        }
        printf("DEBUG: Successfully evaluated right operand\n");

        printf("DEBUG: Evaluating binary operation between types %s and %s\n",
               type_to_string(left->type), type_to_string(right->type));

        ComptimeValue *result = evaluate_comptime_binary_op(expr->data.binary_expr.op, left, right);
        if (!result)
        {
            printf("DEBUG: Binary operation evaluation failed\n");
        }
        else
        {
            printf("DEBUG: Binary operation evaluation succeeded with type %s\n",
                   type_to_string(result->type));
        }

        free_comptime_value(left);
        free_comptime_value(right);
        return result;
    }

    case AST_UNARY_EXPR:
    {
        printf("DEBUG: Evaluating unary expression with operator '%s'\n", expr->data.unary_expr.op);
        ComptimeValue *operand = evaluate_comptime_expr_with_symbols(expr->data.unary_expr.operand, symbols);
        if (!operand)
        {
            printf("DEBUG: Failed to evaluate unary operand\n");
            return NULL;
        }
        printf("DEBUG: Successfully evaluated unary operand\n");

        ComptimeValue *result = evaluate_comptime_unary_op(expr->data.unary_expr.op, operand);
        if (!result)
        {
            printf("DEBUG: Unary operation evaluation failed\n");
        }
        else
        {
            printf("DEBUG: Unary operation evaluation succeeded with type %s\n",
                   type_to_string(result->type));
        }

        free_comptime_value(operand);
        return result;
    }

    case AST_ARRAY_LITERAL:
    {
        if (!expr->data.array_literal.element_type)
        {
            printf("DEBUG: Array literal missing element type\n");
            return NULL;
        }
        return evaluate_array_literal(expr->data.array_literal.elements,
                                      expr->data.array_literal.element_count,
                                      expr->data.array_literal.element_type);
    }

    case AST_ARRAY_INDEX:
    {
        printf("DEBUG: Evaluating array index\n");
        ComptimeValue *array = evaluate_comptime_expr_with_symbols(
            expr->data.array_index.array, symbols);
        if (!array)
        {
            printf("DEBUG: Failed to evaluate array in indexing\n");
            return NULL;
        }

        ComptimeValue *index = evaluate_comptime_expr_with_symbols(
            expr->data.array_index.index, symbols);
        if (!index)
        {
            printf("DEBUG: Failed to evaluate index\n");
            free_comptime_value(array);
            return NULL;
        }

        ComptimeValue *result = evaluate_array_index(array, index);
        free_comptime_value(array);
        free_comptime_value(index);
        return result;
    }

    case AST_FUNC_CALL:
    {
        printf("DEBUG: Evaluating function call to '%s'\n", expr->data.func_call.name);
        if (strcmp(expr->data.func_call.name, "len") == 0)
        {
            if (expr->data.func_call.arg_count != 1)
            {
                printf("DEBUG: len() requires exactly one argument\n");
                return NULL;
            }

            ComptimeValue *array = evaluate_comptime_expr_with_symbols(
                expr->data.func_call.arguments[0], symbols);
            if (!array)
            {
                printf("DEBUG: Failed to evaluate array argument to len()\n");
                return NULL;
            }

            ComptimeValue *result = get_array_length(array);
            free_comptime_value(array);
            return result;
        }

        printf("DEBUG: Function '%s' cannot be evaluated at compile time\n",
               expr->data.func_call.name);
        return NULL;
    }

    default:
        printf("DEBUG: Cannot evaluate expression type %d at compile time\n", expr->type);
        return NULL;
    }
}

// Original evaluation function now calls the new one with NULL symbols
ComptimeValue *evaluate_comptime_expr(ASTNode *expr)
{
    return evaluate_comptime_expr_with_symbols(expr, NULL);
}