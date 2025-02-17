#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

// Perform semantic analysis on the AST. This function creates a global symbol
// table, traverses the AST to perform semantic checks (such as variable
// declaration checks and type consistency), and then cleans up the symbol
// table.
void semantic_analysis(ASTNode *root);

// Recursively visit the AST nodes using the provided symbol table (for the
// current scope). This function performs semantic checks and populates the
// symbol table as needed.
void semantic_visit(ASTNode *node, SymbolTable *table);

#endif // SEMANTIC_H
