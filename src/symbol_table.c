#include "../include/symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16

// Create a new symbol table
SymbolTable *create_symbol_table(SymbolTable *parent)
{
    SymbolTable *table = malloc(sizeof(SymbolTable));
    if (!table)
    {
        fprintf(stderr, "Failed to allocate symbol table\n");
        exit(1);
    }

    table->symbols = malloc(INITIAL_CAPACITY * sizeof(Symbol *));
    if (!table->symbols)
    {
        fprintf(stderr, "Failed to allocate symbol array\n");
        free(table);
        exit(1);
    }

    table->count = 0;
    table->capacity = INITIAL_CAPACITY;
    table->parent = parent;
    return table;
}

// Add a symbol to the table
void add_symbol(SymbolTable *table, const char *name, const char *type)
{
    add_symbol_with_node(table, name, type, NULL);
}

// Add a symbol with an associated AST node
void add_symbol_with_node(SymbolTable *table, const char *name, const char *type, ASTNode *node)
{
    // Check if we need to resize
    if (table->count >= table->capacity)
    {
        table->capacity *= 2;
        table->symbols = realloc(table->symbols, table->capacity * sizeof(Symbol *));
        if (!table->symbols)
        {
            fprintf(stderr, "Failed to resize symbol table\n");
            exit(1);
        }
    }

    // Create new symbol
    Symbol *sym = malloc(sizeof(Symbol));
    if (!sym)
    {
        fprintf(stderr, "Failed to allocate symbol\n");
        exit(1);
    }

    sym->name = strdup(name);
    sym->type = strdup(type);
    sym->node = node;

    if (!sym->name || !sym->type)
    {
        fprintf(stderr, "Failed to allocate symbol strings\n");
        exit(1);
    }

    table->symbols[table->count++] = sym;
}

// Look up a symbol in the current table and parent tables
Symbol *lookup_symbol(SymbolTable *table, const char *name)
{
    while (table)
    {
        for (int i = 0; i < table->count; i++)
        {
            if (strcmp(table->symbols[i]->name, name) == 0)
            {
                return table->symbols[i];
            }
        }
        table = table->parent;
    }
    return NULL;
}

// Free a symbol table and all its symbols
void destroy_symbol_table(SymbolTable *table)
{
    if (!table)
        return;

    for (int i = 0; i < table->count; i++)
    {
        free(table->symbols[i]->name);
        free(table->symbols[i]->type);
        free(table->symbols[i]);
    }

    free(table->symbols);
    free(table);
}
