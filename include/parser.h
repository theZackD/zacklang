#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

// Parser structure to maintain state
typedef struct Parser
{
    TokenArray tokens; // Token stream from lexer
    int current;       // Current token position
    int had_error;     // Error flag
} Parser;

// Create a new parser instance
Parser *create_parser(TokenArray tokens);

// Free parser resources
void destroy_parser(Parser *parser);

// Parse a variable declaration
// let [const] identifier [:type] = expression;
ASTNode *parse_var_declaration(Parser *parser);

// Parse a function declaration
// [comptime] fn identifier(params) [:type] { body }
ASTNode *parse_function_declaration(Parser *parser);

// Parse a single expression
ASTNode *parse_expression(Parser *parser);

// Parse a block of statements
ASTNode *parse_block(Parser *parser);

// Error reporting
void parser_error(Parser *parser, const char *message);

#endif // PARSER_H