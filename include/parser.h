#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include <stdbool.h>

// Parser state structure
typedef struct
{
  TokenArray tokens; // Array of tokens from lexer
  int current;       // Current token index
  int had_error;     // Error flag
  char *error_msg;   // Error message
} Parser;

// Initialize parser with token array
Parser init_parser(TokenArray tokens);

// Parse a complete program
ASTNode *parse_program(Parser *parser);

// Core parsing functions
ASTNode *parse_statement(Parser *parser);
ASTNode *parse_expression(Parser *parser);

// Statement parsing functions
ASTNode *parse_var_decl(Parser *parser);
ASTNode *parse_print_stmt(Parser *parser);
ASTNode *parse_if_stmt(Parser *parser);
ASTNode *parse_while_stmt(Parser *parser);
ASTNode *parse_for_stmt(Parser *parser);
ASTNode *parse_return_stmt(Parser *parser);
ASTNode *parse_block(Parser *parser);
ASTNode *parse_func_def(Parser *parser);

// Expression parsing functions (in order of precedence)
ASTNode *parse_assignment(Parser *parser);
ASTNode *parse_logical_or(Parser *parser);
ASTNode *parse_logical_and(Parser *parser);
ASTNode *parse_equality(Parser *parser);
ASTNode *parse_comparison(Parser *parser);
ASTNode *parse_term(Parser *parser);
ASTNode *parse_factor(Parser *parser);
ASTNode *parse_unary(Parser *parser);
ASTNode *parse_primary(Parser *parser);

// Helper functions
Token *peek(Parser *parser);                                       // Look at current token
Token *advance(Parser *parser);                                    // Move to next token
Token *previous(Parser *parser);                                   // Get previous token
bool is_at_end(Parser *parser);                                    // Check if at end of tokens
bool check(Parser *parser, TokenType type);                        // Check current token type
bool match(Parser *parser, TokenType type);                        // Check and advance if match
void consume(Parser *parser, TokenType type, const char *message); // Expect and consume token
void error(Parser *parser, const char *message);                   // Report error

#endif // PARSER_H
