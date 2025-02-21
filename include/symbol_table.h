#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"

// Symbol structure
typedef struct Symbol
{
    char *name;
    char *type;
    ASTNode *node; // For function definitions and other declarations.
} Symbol;

// Symbol table structure
typedef struct SymbolTable
{
    Symbol **symbols;
    int count;
    int capacity;
    struct SymbolTable *parent; // For nested scopes.
} SymbolTable;

// Create and destroy symbol tables.
SymbolTable *create_symbol_table(SymbolTable *parent);
void destroy_symbol_table(SymbolTable *table);

// Symbol management.
void add_symbol(SymbolTable *table, const char *name, const char *type);
void add_symbol_with_node(SymbolTable *table, const char *name, const char *type, ASTNode *node);
Symbol *lookup_symbol(SymbolTable *table, const char *name);

#endif // SYMBOL_TABLE_H
