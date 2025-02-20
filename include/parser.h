#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

// Parser state structure that holds the token array and the current token
// index.
typedef struct
{
  TokenArray tokens; // Array of tokens produced by the lexer.
  int current;       // Current index in the token array.
} Parser;

// Initializes a Parser with the given TokenArray.
Parser init_parser(TokenArray tokens);

// Returns a pointer to the current token without consuming it.
Token *peek(Parser *parser);

// Advances the parser to the next token and returns the new current token.
Token *advance(Parser *parser);

// If the current token matches the specified type, consumes it and returns 1;
// otherwise, returns 0.
int match(Parser *parser, TokenType type);

// Reports a parsing error (including token context) and terminates the program.
void error(Parser *parser, const char *message);

// Parses an entire program (a sequence of statements) and returns the AST root
// node.
ASTNode *parse_program(Parser *parser);

// Parses a single statement and returns its corresponding AST node.
ASTNode *parse_statement(Parser *parser);

// Parses an expression statement (an expression followed by a semicolon).
ASTNode *parse_expression_stmt(Parser *parser);

// Parses a variable declaration statement: "let" [ "const" ] IDENTIFIER [ ":"
// type ] "=" expression ";".
ASTNode *parse_var_decl(Parser *parser);

// Parses a print statement: "print" "(" expression ")" ";".
ASTNode *parse_print_stmt(Parser *parser);

// Parses a prompt statement: "prompt" "(" expression ")" ";".
ASTNode *parse_prompt_stmt(Parser *parser);

// Parses an if statement: "if" "(" expression ")" block { "elif" "(" expression
// ")" block } [ "else" block ].
ASTNode *parse_if_stmt(Parser *parser);

// Parses a switch statement: "switch" "(" expression ")" "{" { case_stmt } [
// finally_stmt ] "}".
ASTNode *parse_switch_stmt(Parser *parser);

// Parses a while loop: "while" "(" expression ")" block.
ASTNode *parse_while_stmt(Parser *parser);

// Parses a for loop: "for" "(" IDENTIFIER "in" "{" expression ":" expression
// "}" ")" block.
ASTNode *parse_for_stmt(Parser *parser);

// Parses a function definition: [ "comptime" ] "fn" IDENTIFIER "(" [
// parameter_list ] ")" [ ":" type ] block.
ASTNode *parse_func_def(Parser *parser);

// Parses a break statement: "break" ";".
ASTNode *parse_break_stmt(Parser *parser);

// Parses a continue statement: "continue" ";".
ASTNode *parse_continue_stmt(Parser *parser);

// Parses a return statement: "return" [ expression ] ";".
ASTNode *parse_return_stmt(Parser *parser);

// Parses a block: "{" { statement } "}".
ASTNode *parse_block(Parser *parser);

/*
 * Expression Parsing:
 * These functions implement recursive descent parsing with proper operator
 * precedence.
 */

// Entry point for expression parsing.
ASTNode *parse_expression(Parser *parser);

// Parses an assignment expression (right-associative).
ASTNode *parse_assignment(Parser *parser);

// Parses a logical OR expression.
ASTNode *parse_logical_or(Parser *parser);

// Parses a logical AND expression.
ASTNode *parse_logical_and(Parser *parser);

// Parses an equality expression (==, !=).
ASTNode *parse_equality(Parser *parser);

// Parses a relational expression (<, >, <=, >=).
ASTNode *parse_relational(Parser *parser);

// Parses an additive expression (+, -).
ASTNode *parse_additive(Parser *parser);

// Parses a multiplicative expression (*, /, %).
ASTNode *parse_multiplicative(Parser *parser);

// Parses a unary expression (-, +, not).
ASTNode *parse_unary(Parser *parser);

// Parses a primary expression: identifiers, literals, grouped expressions, or
// function calls.
ASTNode *parse_primary(Parser *parser);

#endif // PARSER_H
