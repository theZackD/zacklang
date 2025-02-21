#include "../include/symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16

// Memory allocation helper functions.
static void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        fprintf(stderr, "Memory allocation failed for %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static char *xstrdup(const char *s)
{
    char *ptr = strdup(s);
    if (!ptr)
    {
        fprintf(stderr, "Memory allocation failed in strdup\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Create a new symbol table. The 'parent' pointer allows nested scopes.
SymbolTable *create_symbol_table(SymbolTable *parent)
{
    SymbolTable *table = (SymbolTable *)xmalloc(sizeof(SymbolTable));
    table->symbols = (Symbol **)xmalloc(INITIAL_CAPACITY * sizeof(Symbol *));
    table->count = 0;
    table->capacity = INITIAL_CAPACITY;
    table->parent = parent;
    return table;
}

// Add a symbol to the table (without an associated AST node).
void add_symbol(SymbolTable *table, const char *name, const char *type)
{
    add_symbol_with_node(table, name, type, NULL);
}

// Add a symbol along with an associated AST node (for functions, structs, etc.).
void add_symbol_with_node(SymbolTable *table, const char *name, const char *type, ASTNode *node)
{
    // Resize the symbol array if needed.
    if (table->count >= table->capacity)
    {
        table->capacity *= 2;
        table->symbols = realloc(table->symbols, table->capacity * sizeof(Symbol *));
        if (!table->symbols)
        {
            fprintf(stderr, "Failed to resize symbol table\n");
            exit(EXIT_FAILURE);
        }
    }

    // Allocate and initialize a new symbol.
    Symbol *sym = (Symbol *)xmalloc(sizeof(Symbol));
    sym->name = xstrdup(name);
    sym->type = xstrdup(type);
    sym->node = node;

    table->symbols[table->count++] = sym;
}

// Look up a symbol in the current table; if not found, search parent tables.
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

// Destroy a symbol table and free all its symbols.
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
