#include "include/symbol_table.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_symbol_table_creation()
{
    printf("\nTesting symbol table creation...\n");

    SymbolTable *table = create_symbol_table(NULL);
    assert(table != NULL);
    assert(table->count == 0);
    assert(table->parent == NULL);

    destroy_symbol_table(table);
    printf("✓ Symbol table creation test passed\n");
}

void test_symbol_addition()
{
    printf("\nTesting symbol addition...\n");

    SymbolTable *table = create_symbol_table(NULL);

    // Test adding a new symbol
    assert(add_symbol(table, "x", "i32") == 1);
    assert(table->count == 1);
    assert(strcmp(table->symbols[0]->name, "x") == 0);
    assert(strcmp(table->symbols[0]->type, "i32") == 0);

    // Test adding a duplicate symbol (should fail)
    assert(add_symbol(table, "x", "f64") == 0);
    assert(table->count == 1);

    // Test adding another unique symbol
    assert(add_symbol(table, "y", "bool") == 1);
    assert(table->count == 2);

    destroy_symbol_table(table);
    printf("✓ Symbol addition test passed\n");
}

void test_symbol_lookup()
{
    printf("\nTesting symbol lookup...\n");

    // Create parent scope
    SymbolTable *parent = create_symbol_table(NULL);
    add_symbol(parent, "global", "string");

    // Create child scope
    SymbolTable *child = create_symbol_table(parent);
    add_symbol(child, "local", "i32");

    // Test lookup in current scope
    Symbol *local = lookup_symbol(child, "local");
    assert(local != NULL);
    assert(strcmp(local->name, "local") == 0);
    assert(strcmp(local->type, "i32") == 0);

    // Test lookup in parent scope
    Symbol *global = lookup_symbol(child, "global");
    assert(global != NULL);
    assert(strcmp(global->name, "global") == 0);
    assert(strcmp(global->type, "string") == 0);

    // Test lookup of non-existent symbol
    Symbol *nonexistent = lookup_symbol(child, "nonexistent");
    assert(nonexistent == NULL);

    destroy_symbol_table(child);
    destroy_symbol_table(parent);
    printf("✓ Symbol lookup test passed\n");
}

void test_nested_scopes()
{
    printf("\nTesting nested scopes...\n");

    // Create three levels of scope
    SymbolTable *global = create_symbol_table(NULL);
    SymbolTable *function = create_symbol_table(global);
    SymbolTable *block = create_symbol_table(function);

    // Add symbols at each level
    add_symbol(global, "pi", "f64");
    add_symbol(function, "x", "i32");
    add_symbol(block, "temp", "i32");

    // Test shadowing
    add_symbol(block, "x", "f64"); // Should succeed as it's in a different scope

    // Verify lookups at different levels
    Symbol *pi = lookup_symbol(block, "pi");
    assert(pi != NULL);
    assert(strcmp(pi->type, "f64") == 0);

    Symbol *x = lookup_symbol(block, "x");
    assert(x != NULL);
    assert(strcmp(x->type, "f64") == 0); // Should find the shadowed version

    Symbol *temp = lookup_symbol(block, "temp");
    assert(temp != NULL);
    assert(strcmp(temp->type, "i32") == 0);

    destroy_symbol_table(block);
    destroy_symbol_table(function);
    destroy_symbol_table(global);
    printf("✓ Nested scopes test passed\n");
}

int main()
{
    printf("Running symbol table tests...\n");

    test_symbol_table_creation();
    test_symbol_addition();
    test_symbol_lookup();
    test_nested_scopes();

    printf("\nAll symbol table tests passed! ✓\n");
    return 0;
}