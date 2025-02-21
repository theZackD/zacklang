#include "../include/variable_tracking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//------------------------------------------------------
// Memory Allocation Helpers
//------------------------------------------------------
static void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        fprintf(stderr, "Error: Failed to allocate %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static void *xcalloc(size_t count, size_t size)
{
    void *ptr = calloc(count, size);
    if (!ptr)
    {
        fprintf(stderr, "Error: Failed to allocate %zu bytes (calloc)\n", count * size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static void *xrealloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr)
    {
        fprintf(stderr, "Error: Failed to reallocate to %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

static char *xstrdup(const char *s)
{
    char *dup = strdup(s);
    if (!dup)
    {
        fprintf(stderr, "Error: Failed to duplicate string\n");
        exit(EXIT_FAILURE);
    }
    return dup;
}

//------------------------------------------------------
// Constants
//------------------------------------------------------
#define INITIAL_DEF_CAPACITY 4

//------------------------------------------------------
// Hash function for variable names.
//------------------------------------------------------
static unsigned int hash_string(const char *str)
{
    unsigned int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash;
}

//------------------------------------------------------
// Create a new variable version entry.
//------------------------------------------------------
static VariableVersion *create_variable_version(const char *name)
{
    VariableVersion *var = (VariableVersion *)xmalloc(sizeof(VariableVersion));
    var->name = xstrdup(name);
    var->current_version = -1; // No versions yet.
    var->def_capacity = INITIAL_DEF_CAPACITY;
    var->definitions = (ZIRValue **)xmalloc(sizeof(ZIRValue *) * var->def_capacity);
    var->next = NULL;
    return var;
}

//------------------------------------------------------
// Create a new variable table.
//------------------------------------------------------
VariableTable *create_variable_table(int initial_buckets)
{
    VariableTable *table = (VariableTable *)xmalloc(sizeof(VariableTable));
    table->buckets = (VariableVersion **)xcalloc(initial_buckets, sizeof(VariableVersion *));
    table->bucket_count = initial_buckets;
    table->total_variables = 0;
    return table;
}

//------------------------------------------------------
// Retrieve a variable version entry from the table.
//------------------------------------------------------
static VariableVersion *get_variable(VariableTable *table, const char *name)
{
    if (!table || !name)
        return NULL;
    unsigned int bucket = hash_string(name) % table->bucket_count;
    VariableVersion *var = table->buckets[bucket];
    while (var)
    {
        if (strcmp(var->name, name) == 0)
            return var;
        var = var->next;
    }
    return NULL;
}

//------------------------------------------------------
// Add a new variable to the table.
//------------------------------------------------------
static VariableVersion *add_variable(VariableTable *table, const char *name)
{
    if (!table || !name)
        return NULL;
    unsigned int bucket = hash_string(name) % table->bucket_count;
    VariableVersion *var = create_variable_version(name);
    var->next = table->buckets[bucket];
    table->buckets[bucket] = var;
    table->total_variables++;
    return var;
}

//------------------------------------------------------
// Get current version of a variable.
//------------------------------------------------------
int get_current_version(VariableTable *table, const char *name)
{
    VariableVersion *var = get_variable(table, name);
    return var ? var->current_version : -1;
}

//------------------------------------------------------
// Create a new version for a variable.
//------------------------------------------------------
int create_new_version(VariableTable *table, const char *name, ZIRValue *definition)
{
    if (!table || !name)
        return -1;
    VariableVersion *var = get_variable(table, name);
    if (!var)
        var = add_variable(table, name);
    // Resize definitions array if needed.
    if (var->current_version + 1 >= var->def_capacity)
    {
        int new_capacity = var->def_capacity * 2;
        var->definitions = (ZIRValue **)xrealloc(var->definitions, sizeof(ZIRValue *) * new_capacity);
        var->def_capacity = new_capacity;
    }
    var->current_version++;
    var->definitions[var->current_version] = definition;
    return var->current_version;
}

//------------------------------------------------------
// Get definition for a specific version.
//------------------------------------------------------
ZIRValue *get_version_definition(VariableTable *table, const char *name, int version)
{
    VariableVersion *var = get_variable(table, name);
    if (!var || version < 0 || version > var->current_version)
        return NULL;
    return var->definitions[version];
}

//------------------------------------------------------
// Get the current definition of a variable.
//------------------------------------------------------
ZIRValue *get_current_definition(VariableTable *table, const char *name)
{
    VariableVersion *var = get_variable(table, name);
    if (!var || var->current_version < 0)
        return NULL;
    return var->definitions[var->current_version];
}

//------------------------------------------------------
// Check if a variable exists in the table.
//------------------------------------------------------
bool has_variable(VariableTable *table, const char *name)
{
    return get_variable(table, name) != NULL;
}

//------------------------------------------------------
// Get total number of versions for a variable.
//------------------------------------------------------
int get_version_count(VariableTable *table, const char *name)
{
    VariableVersion *var = get_variable(table, name);
    return var ? var->current_version + 1 : 0;
}

//------------------------------------------------------
// Print all variable versions (for debugging).
//------------------------------------------------------
void print_variable_versions(VariableTable *table)
{
    if (!table)
        return;
    printf("Variable Version Table:\n");
    for (int i = 0; i < table->bucket_count; i++)
    {
        VariableVersion *var = table->buckets[i];
        while (var)
        {
            printf("Variable: %s\n", var->name);
            printf("  Current Version: %d\n", var->current_version);
            printf("  Versions: ");
            for (int v = 0; v <= var->current_version; v++)
            {
                printf("%d ", v);
            }
            printf("\n");
            var = var->next;
        }
    }
}

//------------------------------------------------------
// Destroy the variable table and free all associated memory.
//------------------------------------------------------
void destroy_variable_table(VariableTable *table)
{
    if (!table)
        return;
    for (int i = 0; i < table->bucket_count; i++)
    {
        VariableVersion *var = table->buckets[i];
        while (var)
        {
            VariableVersion *next = var->next;
            free(var->name);
            free(var->definitions);
            free(var);
            var = next;
        }
    }
    free(table->buckets);
    free(table);
}
