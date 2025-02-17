#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>
#include "ast.h"

// A Symbol represents a declared identifier (e.g., variable or function)
// along with its associated type.
typedef struct Symbol
{
    char *name;
    char *type;    // For now, we store type information as a string.
    ASTNode *node; // For functions, store the AST node for parameter checking
    // Additional fields (like scope level, memory location, etc.) can be added here.
} Symbol;

// A SymbolTable holds an array of pointers to Symbols and supports nested scopes
// via a pointer to a parent table.
typedef struct SymbolTable
{
    Symbol **symbols;
    int count;
    int capacity;
    struct SymbolTable *parent; // NULL if global scope.
} SymbolTable;

// Creates a new symbol table with an optional parent scope.
SymbolTable *create_symbol_table(SymbolTable *parent);

// Adds a symbol with the given name and type to the table.
// Returns 1 on success, or 0 if the symbol already exists in the current scope.
int add_symbol(SymbolTable *table, const char *name, const char *type);

// Adds a symbol with the given name, type, and AST node to the table.
// This is used for functions where we need to store the function definition.
int add_symbol_with_node(SymbolTable *table, const char *name, const char *type, ASTNode *node);

// Looks up a symbol by name in the current table and, if not found, recursively in parent tables.
// Returns a pointer to the Symbol if found, or NULL if not.
Symbol *lookup_symbol(SymbolTable *table, const char *name);

// Frees all memory associated with the symbol table, including all stored symbols.
void destroy_symbol_table(SymbolTable *table);

#endif // SYMBOL_TABLE_H
