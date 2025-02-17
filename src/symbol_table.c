#include "../include/symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

SymbolTable *create_symbol_table(SymbolTable *parent)
{
    SymbolTable *table = malloc(sizeof(SymbolTable));
    if (!table)
    {
        fprintf(stderr, "Failed to allocate memory for symbol table\n");
        exit(1);
    }

    table->symbols = malloc(INITIAL_CAPACITY * sizeof(Symbol *));
    if (!table->symbols)
    {
        free(table);
        fprintf(stderr, "Failed to allocate memory for symbols array\n");
        exit(1);
    }

    table->count = 0;
    table->capacity = INITIAL_CAPACITY;
    table->parent = parent;
    return table;
}

int add_symbol(SymbolTable *table, const char *name, const char *type)
{
    return add_symbol_with_node(table, name, type, NULL);
}

int add_symbol_with_node(SymbolTable *table, const char *name, const char *type, ASTNode *node)
{
    // First check if symbol already exists in current scope
    for (int i = 0; i < table->count; i++)
    {
        if (strcmp(table->symbols[i]->name, name) == 0)
        {
            return 0; // Symbol already exists in current scope
        }
    }

    // Create new symbol
    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol)
    {
        fprintf(stderr, "Failed to allocate memory for symbol\n");
        exit(1);
    }

    symbol->name = strdup(name);
    symbol->type = strdup(type);
    symbol->node = node;

    // Resize symbols array if needed
    if (table->count >= table->capacity)
    {
        table->capacity *= 2;
        Symbol **temp = realloc(table->symbols, table->capacity * sizeof(Symbol *));
        if (!temp)
        {
            free(symbol->name);
            free(symbol->type);
            free(symbol);
            fprintf(stderr, "Failed to reallocate memory for symbols array\n");
            exit(1);
        }
        table->symbols = temp;
    }

    // Add symbol to table
    table->symbols[table->count++] = symbol;
    return 1;
}

Symbol *lookup_symbol(SymbolTable *table, const char *name)
{
    // Search in current scope
    for (int i = 0; i < table->count; i++)
    {
        if (strcmp(table->symbols[i]->name, name) == 0)
        {
            return table->symbols[i];
        }
    }

    // If not found and we have a parent scope, search there
    if (table->parent)
    {
        return lookup_symbol(table->parent, name);
    }

    return NULL; // Symbol not found
}

void destroy_symbol_table(SymbolTable *table)
{
    if (!table)
        return;

    // Free all symbols
    for (int i = 0; i < table->count; i++)
    {
        free(table->symbols[i]->name);
        free(table->symbols[i]->type);
        // Note: We don't free the node as it's part of the AST
        free(table->symbols[i]);
    }

    // Free the symbols array and table itself
    free(table->symbols);
    free(table);
}
