// Variable tracking for SSA form
#ifndef VARIABLE_TRACKING_H
#define VARIABLE_TRACKING_H

#include "zir.h"
#include <stdbool.h>

// Structure to track a single variable's versions
typedef struct VariableVersion
{
    char *name;                   // Variable name
    int current_version;          // Current version number
    ZIRValue **definitions;       // Array of definitions (one per version)
    int def_capacity;             // Capacity of definitions array
    struct VariableVersion *next; // For hash table chaining
} VariableVersion;

// Hash table to track all variables
typedef struct VariableTable
{
    VariableVersion **buckets;
    int bucket_count;
    int total_variables;
} VariableTable;

// Create and destroy variable table
VariableTable *create_variable_table(int initial_buckets);
void destroy_variable_table(VariableTable *table);

// Variable version tracking
int get_current_version(VariableTable *table, const char *name);
int create_new_version(VariableTable *table, const char *name, ZIRValue *definition);
ZIRValue *get_version_definition(VariableTable *table, const char *name, int version);
ZIRValue *get_current_definition(VariableTable *table, const char *name);

// Utility functions
void print_variable_versions(VariableTable *table);
bool has_variable(VariableTable *table, const char *name);
int get_version_count(VariableTable *table, const char *name);

#endif // VARIABLE_TRACKING_H