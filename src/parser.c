#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Initialize parser
Parser init_parser(TokenArray tokens)
{
  Parser parser = {
      .tokens = tokens,
      .current = 0,
      .had_error = 0,
      .error_msg = NULL};
  return parser;
}

// Helper functions
Token *peek(Parser *parser)
{
  return &parser->tokens.tokens[parser->current];
}

Token *advance(Parser *parser)
{
  if (!is_at_end(parser))
    parser->current++;
  return previous(parser);
}

Token *previous(Parser *parser)
{
  return &parser->tokens.tokens[parser->current - 1];
}

bool is_at_end(Parser *parser)
{
  return peek(parser)->type == TOKEN_EOF;
}

bool check(Parser *parser, TokenType type)
{
  if (is_at_end(parser))
    return false;
  return peek(parser)->type == type;
}

bool match(Parser *parser, TokenType type)
{
  if (check(parser, type))
  {
    advance(parser);
    return true;
  }
  return false;
}

void consume(Parser *parser, TokenType type, const char *message)
{
  if (check(parser, type))
  {
    advance(parser);
    return;
  }

  error(parser, message);
}

void error(Parser *parser, const char *message)
{
  if (parser->error_msg)
    free(parser->error_msg);
  parser->error_msg = strdup(message);
  parser->had_error = 1;
}

// Synchronize after error
static void synchronize(Parser *parser)
{
  advance(parser);

  while (!is_at_end(parser))
  {
    if (previous(parser)->type == TOKEN_SEMICOLON)
      return;

    switch (peek(parser)->type)
    {
    case TOKEN_KEYWORD:
      // Synchronize on statement keywords
      if (strcmp(peek(parser)->value, "fn") == 0 ||
          strcmp(peek(parser)->value, "let") == 0 ||
          strcmp(peek(parser)->value, "if") == 0 ||
          strcmp(peek(parser)->value, "while") == 0 ||
          strcmp(peek(parser)->value, "return") == 0)
      {
        return;
      }
      break;
    default:
      break;
    }

    advance(parser);
  }
}

// Parse a complete program
ASTNode *parse_program(Parser *parser)
{
  // Create a block to hold all top-level statements
  ASTNode **statements = NULL;
  int stmt_count = 0;
  int capacity = 0;

  while (!is_at_end(parser))
  {
    // Expand statements array if needed
    if (stmt_count >= capacity)
    {
      capacity = capacity == 0 ? 8 : capacity * 2;
      statements = realloc(statements, capacity * sizeof(ASTNode *));
    }

    // Parse next statement
    ASTNode *stmt = parse_statement(parser);
    if (stmt)
    {
      statements[stmt_count++] = stmt;
    }

    // Handle error recovery
    if (parser->had_error)
    {
      synchronize(parser);
      parser->had_error = 0;
      free(parser->error_msg);
      parser->error_msg = NULL;
    }
  }

  // Create and return block node
  return create_block(statements, stmt_count);
}

// Parse a statement
ASTNode *parse_statement(Parser *parser)
{
  Token *token = peek(parser);

  // Check for keywords first
  if (token->type == TOKEN_KEYWORD)
  {
    if (strcmp(token->value, "let") == 0)
    {
      return parse_var_decl(parser);
    }
    if (strcmp(token->value, "fn") == 0)
    {
      return parse_func_def(parser);
    }
    if (strcmp(token->value, "if") == 0)
    {
      return parse_if_stmt(parser);
    }
    if (strcmp(token->value, "while") == 0)
    {
      return parse_while_stmt(parser);
    }
    if (strcmp(token->value, "for") == 0)
    {
      return parse_for_stmt(parser);
    }
    if (strcmp(token->value, "return") == 0)
    {
      return parse_return_stmt(parser);
    }
    if (strcmp(token->value, "print") == 0)
    {
      return parse_print_stmt(parser);
    }
  }

  // If not a keyword, must be an expression statement
  ASTNode *expr = parse_expression(parser);
  consume(parser, TOKEN_SEMICOLON, "Expected ';' after expression.");
  return create_expr_stmt(expr);
}

// Expression parsing functions in order of precedence
ASTNode *parse_assignment(Parser *parser)
{
  ASTNode *expr = parse_logical_or(parser);

  if (match(parser, TOKEN_OPERATOR) && strcmp(previous(parser)->value, "=") == 0)
  {
    ASTNode *value = parse_assignment(parser); // Right associative

    // Validate assignment target
    if (expr->type == AST_IDENTIFIER)
    {
      return create_assign_expr(expr, value);
    }

    error(parser, "Invalid assignment target");
  }

  return expr;
}

ASTNode *parse_logical_or(Parser *parser)
{
  ASTNode *expr = parse_logical_and(parser);

  while (match(parser, TOKEN_KEYWORD) && strcmp(previous(parser)->value, "or") == 0)
  {
    ASTNode *right = parse_logical_and(parser);
    expr = create_binary_expr("or", expr, right);
  }

  return expr;
}

ASTNode *parse_logical_and(Parser *parser)
{
  ASTNode *expr = parse_equality(parser);

  while (match(parser, TOKEN_KEYWORD) && strcmp(previous(parser)->value, "and") == 0)
  {
    ASTNode *right = parse_equality(parser);
    expr = create_binary_expr("and", expr, right);
  }

  return expr;
}

ASTNode *parse_equality(Parser *parser)
{
  ASTNode *expr = parse_comparison(parser);

  while (match(parser, TOKEN_OPERATOR) &&
         (strcmp(previous(parser)->value, "==") == 0 ||
          strcmp(previous(parser)->value, "!=") == 0))
  {
    char *op = strdup(previous(parser)->value);
    ASTNode *right = parse_comparison(parser);
    expr = create_binary_expr(op, expr, right);
    free(op);
  }

  return expr;
}

ASTNode *parse_comparison(Parser *parser)
{
  ASTNode *expr = parse_term(parser);

  while (match(parser, TOKEN_OPERATOR) &&
         (strcmp(previous(parser)->value, "<") == 0 ||
          strcmp(previous(parser)->value, ">") == 0 ||
          strcmp(previous(parser)->value, "<=") == 0 ||
          strcmp(previous(parser)->value, ">=") == 0))
  {
    char *op = strdup(previous(parser)->value);
    ASTNode *right = parse_term(parser);
    expr = create_binary_expr(op, expr, right);
    free(op);
  }

  return expr;
}

ASTNode *parse_term(Parser *parser)
{
  ASTNode *expr = parse_factor(parser);

  while (match(parser, TOKEN_OPERATOR) &&
         (strcmp(previous(parser)->value, "+") == 0 ||
          strcmp(previous(parser)->value, "-") == 0))
  {
    char *op = strdup(previous(parser)->value);
    ASTNode *right = parse_factor(parser);
    expr = create_binary_expr(op, expr, right);
    free(op);
  }

  return expr;
}

ASTNode *parse_factor(Parser *parser)
{
  ASTNode *expr = parse_unary(parser);

  while (match(parser, TOKEN_OPERATOR) &&
         (strcmp(previous(parser)->value, "*") == 0 ||
          strcmp(previous(parser)->value, "/") == 0 ||
          strcmp(previous(parser)->value, "%") == 0 ||
          strcmp(previous(parser)->value, "**") == 0))
  {
    char *op = strdup(previous(parser)->value);
    ASTNode *right = parse_unary(parser);
    expr = create_binary_expr(op, expr, right);
    free(op);
  }

  return expr;
}

ASTNode *parse_unary(Parser *parser)
{
  if (match(parser, TOKEN_OPERATOR) &&
      (strcmp(previous(parser)->value, "-") == 0 ||
       strcmp(previous(parser)->value, "+") == 0))
  {
    char *op = strdup(previous(parser)->value);
    ASTNode *right = parse_unary(parser);
    ASTNode *expr = create_unary_expr(op, right);
    free(op);
    return expr;
  }

  if (match(parser, TOKEN_KEYWORD) && strcmp(previous(parser)->value, "not") == 0)
  {
    ASTNode *right = parse_unary(parser);
    return create_unary_expr("not", right);
  }

  return parse_primary(parser);
}

ASTNode *parse_primary(Parser *parser)
{
  if (match(parser, TOKEN_INTEGER) || match(parser, TOKEN_FLOAT) ||
      match(parser, TOKEN_STRING))
  {
    return create_literal(previous(parser)->value);
  }

  if (match(parser, TOKEN_KEYWORD))
  {
    if (strcmp(previous(parser)->value, "true") == 0 ||
        strcmp(previous(parser)->value, "false") == 0)
    {
      return create_literal(previous(parser)->value);
    }
  }

  if (match(parser, TOKEN_IDENTIFIER))
  {
    char *name = strdup(previous(parser)->value);

    // Check for function call
    if (match(parser, TOKEN_LPAREN))
    {
      ASTNode **arguments = NULL;
      int arg_count = 0;

      // Parse arguments
      if (!check(parser, TOKEN_RPAREN))
      {
        do
        {
          // Expand arguments array
          arguments = realloc(arguments, (arg_count + 1) * sizeof(ASTNode *));
          arguments[arg_count++] = parse_expression(parser);
        } while (match(parser, TOKEN_COMMA));
      }

      consume(parser, TOKEN_RPAREN, "Expected ')' after arguments");
      ASTNode *call = create_func_call(name, arguments, arg_count);
      free(name);
      return call;
    }

    // Simple identifier
    ASTNode *id = create_identifier(name);
    free(name);
    return id;
  }

  if (match(parser, TOKEN_LPAREN))
  {
    ASTNode *expr = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
    return expr;
  }

  error(parser, "Expected expression");
  return NULL;
}

// --- Utility Functions ---

Token *peek(Parser *parser) { return &parser->tokens.tokens[parser->current]; }

Token *advance(Parser *parser)
{
  if (parser->current < parser->tokens.count)
    parser->current++;
  return peek(parser);
}

int match(Parser *parser, TokenType type)
{
  if (peek(parser)->type == type)
  {
    advance(parser);
    return 1;
  }
  return 0;
}

void error(Parser *parser, const char *message)
{
  Token *token = peek(parser);
  fprintf(stderr, "Parser error at token %d (value: '%s'): %s\n",
          parser->current, token->value, message);
  exit(1);
}

// --- Top-Level Program Parsing ---

ASTNode *parse_program(Parser *parser)
{
  int capacity = 8;
  int count = 0;
  ASTNode **statements = malloc(capacity * sizeof(ASTNode *));
  if (!statements)
    error(parser, "Memory allocation failed for program statements");

  while (peek(parser)->type != TOKEN_EOF)
  {
    ASTNode *stmt = parse_statement(parser);
    if (!stmt)
      break;
    if (count >= capacity)
    {
      capacity *= 2;
      ASTNode **temp = realloc(statements, capacity * sizeof(ASTNode *));
      if (!temp)
      {
        free(statements);
        error(parser, "Memory reallocation failed for program statements");
      }
      statements = temp;
    }
    statements[count++] = stmt;
  }
  ASTNode *programNode = create_block(statements, count);
  return programNode;
}

// --- Statement Parsing Functions ---

ASTNode *parse_statement(Parser *parser)
{
  Token *token = peek(parser);
  if (token->type == TOKEN_KEYWORD)
  {
    if (strcmp(token->value, "let") == 0)
      return parse_var_decl(parser);
    else if (strcmp(token->value, "print") == 0)
      return parse_print_stmt(parser);
    else if (strcmp(token->value, "prompt") == 0)
      return parse_prompt_stmt(parser);
    else if (strcmp(token->value, "if") == 0)
      return parse_if_stmt(parser);
    else if (strcmp(token->value, "switch") == 0)
      return parse_switch_stmt(parser);
    else if (strcmp(token->value, "while") == 0)
      return parse_while_stmt(parser);
    else if (strcmp(token->value, "for") == 0)
      return parse_for_stmt(parser);
    else if (strcmp(token->value, "fn") == 0 ||
             strcmp(token->value, "comptime") == 0)
      return parse_func_def(parser);
    else if (strcmp(token->value, "break") == 0)
      return parse_break_stmt(parser);
    else if (strcmp(token->value, "continue") == 0)
      return parse_continue_stmt(parser);
    else if (strcmp(token->value, "return") == 0)
      return parse_return_stmt(parser);
    else
      error(parser, "Unexpected keyword in statement");
  }
  else if (token->type == TOKEN_LBRACE)
  {
    return parse_block(parser);
  }
  return parse_expression_stmt(parser);
}

// Parse a return statement: "return" [ expression ] ";"
ASTNode *parse_return_stmt(Parser *parser)
{
  if (peek(parser)->type != TOKEN_KEYWORD ||
      strcmp(peek(parser)->value, "return") != 0)
    error(parser, "Expected 'return' keyword");
  advance(parser); // consume "return"

  // Optional expression
  ASTNode *expr = NULL;
  if (peek(parser)->type != TOKEN_SEMICOLON)
  {
    expr = parse_expression(parser);
  }

  if (!match(parser, TOKEN_SEMICOLON))
    error(parser, "Expected ';' after return statement");

  return create_return_stmt(expr);
}

// --- Print Statement ---
// Syntax: "print" "(" expression ")" ";"
ASTNode *parse_print_stmt(Parser *parser)
{
  // Ensure the current token is "print"
  if (peek(parser)->type != TOKEN_KEYWORD || strcmp(peek(parser)->value, "print") != 0)
    error(parser, "Expected 'print' keyword");
  advance(parser); // consume "print"

  // Expect '('
  if (!match(parser, TOKEN_LPAREN))
    error(parser, "Expected '(' after 'print'");
  ASTNode *expr = parse_expression(parser);
  if (!match(parser, TOKEN_RPAREN))
    error(parser, "Expected ')' after print expression");
  // Expect a semicolon at the end.
  if (!match(parser, TOKEN_SEMICOLON))
    error(parser, "Expected ';' after print statement");

  return create_print_stmt(expr);
}

// --- Prompt Statement ---
// Syntax: "prompt" "(" expression ")" ";"
ASTNode *parse_prompt_stmt(Parser *parser)
{
  // Ensure the current token is "prompt"
  if (peek(parser)->type != TOKEN_KEYWORD || strcmp(peek(parser)->value, "prompt") != 0)
    error(parser, "Expected 'prompt' keyword");
  advance(parser); // consume "prompt"

  // Expect '('
  if (!match(parser, TOKEN_LPAREN))
    error(parser, "Expected '(' after 'prompt'");
  ASTNode *expr = parse_expression(parser);
  if (!match(parser, TOKEN_RPAREN))
    error(parser, "Expected ')' after prompt expression");
  // Expect a semicolon.
  if (!match(parser, TOKEN_SEMICOLON))
    error(parser, "Expected ';' after prompt statement");

  return create_prompt_stmt(expr);
}

ASTNode *parse_expression_stmt(Parser *parser)
{
  ASTNode *expr = parse_expression(parser);
  // If we're not at the end of a block, require a semicolon
  if (peek(parser)->type != TOKEN_RBRACE)
  {
    if (!match(parser, TOKEN_SEMICOLON))
      error(parser, "Expected ';' after expression");
  }
  return create_expr_stmt(expr);
}

// --- Variable Declaration ---

ASTNode *parse_var_decl(Parser *parser)
{
  consume(parser, TOKEN_KEYWORD, "Expected 'let' keyword");

  // Check for const
  int is_const = 0;
  if (match(parser, TOKEN_KEYWORD) && strcmp(previous(parser)->value, "const") == 0)
  {
    is_const = 1;
  }

  // Parse identifier
  if (!match(parser, TOKEN_IDENTIFIER))
  {
    error(parser, "Expected variable name");
    return NULL;
  }
  char *name = strdup(previous(parser)->value);

  // Parse optional type annotation
  char *type = NULL;
  if (match(parser, TOKEN_COLON))
  {
    if (!match(parser, TOKEN_KEYWORD))
    {
      error(parser, "Expected type after ':'");
      free(name);
      return NULL;
    }
    type = strdup(previous(parser)->value);
  }

  // Parse initializer
  consume(parser, TOKEN_OPERATOR, "Expected '=' after variable declaration");
  if (strcmp(previous(parser)->value, "=") != 0)
  {
    error(parser, "Expected '=' after variable declaration");
    free(name);
    free(type);
    return NULL;
  }

  ASTNode *initializer = parse_expression(parser);
  if (!initializer)
  {
    free(name);
    free(type);
    return NULL;
  }

  consume(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration");

  return create_var_decl(is_const, name, type, initializer);
}

// --- If Statement ---

ASTNode *parse_if_stmt(Parser *parser)
{
  consume(parser, TOKEN_KEYWORD, "Expected 'if' keyword");
  consume(parser, TOKEN_LPAREN, "Expected '(' after 'if'");

  ASTNode *condition = parse_expression(parser);
  if (!condition)
  {
    error(parser, "Expected condition in if statement");
    return NULL;
  }

  consume(parser, TOKEN_RPAREN, "Expected ')' after condition");

  ASTNode *if_block = parse_block(parser);
  if (!if_block)
  {
    free_ast(condition);
    return NULL;
  }

  // Parse elif blocks
  ASTNode **elif_conds = NULL;
  ASTNode **elif_blocks = NULL;
  int elif_count = 0;

  while (match(parser, TOKEN_KEYWORD) && strcmp(previous(parser)->value, "elif") == 0)
  {
    consume(parser, TOKEN_LPAREN, "Expected '(' after 'elif'");

    ASTNode *elif_cond = parse_expression(parser);
    if (!elif_cond)
    {
      error(parser, "Expected condition in elif statement");
      free_ast(condition);
      free_ast(if_block);
      for (int i = 0; i < elif_count; i++)
      {
        free_ast(elif_conds[i]);
        free_ast(elif_blocks[i]);
      }
      free(elif_conds);
      free(elif_blocks);
      return NULL;
    }

    consume(parser, TOKEN_RPAREN, "Expected ')' after elif condition");

    ASTNode *elif_block = parse_block(parser);
    if (!elif_block)
    {
      free_ast(elif_cond);
      free_ast(condition);
      free_ast(if_block);
      for (int i = 0; i < elif_count; i++)
      {
        free_ast(elif_conds[i]);
        free_ast(elif_blocks[i]);
      }
      free(elif_conds);
      free(elif_blocks);
      return NULL;
    }

    elif_conds = realloc(elif_conds, (elif_count + 1) * sizeof(ASTNode *));
    elif_blocks = realloc(elif_blocks, (elif_count + 1) * sizeof(ASTNode *));
    elif_conds[elif_count] = elif_cond;
    elif_blocks[elif_count] = elif_block;
    elif_count++;
  }

  // Parse optional else block
  ASTNode *else_block = NULL;
  if (match(parser, TOKEN_KEYWORD) && strcmp(previous(parser)->value, "else") == 0)
  {
    else_block = parse_block(parser);
    if (!else_block)
    {
      free_ast(condition);
      free_ast(if_block);
      for (int i = 0; i < elif_count; i++)
      {
        free_ast(elif_conds[i]);
        free_ast(elif_blocks[i]);
      }
      free(elif_conds);
      free(elif_blocks);
      return NULL;
    }
  }

  return create_if_stmt(condition, if_block, elif_conds, elif_blocks, elif_count, else_block);
}

// --- Switch Statement ---

ASTNode *parse_switch_stmt(Parser *parser)
{
  error(parser, "Switch statement parsing not implemented");
  return NULL;
}

// --- While Loop ---

ASTNode *parse_while_stmt(Parser *parser)
{
  consume(parser, TOKEN_KEYWORD, "Expected 'while' keyword");
  consume(parser, TOKEN_LPAREN, "Expected '(' after 'while'");

  ASTNode *condition = parse_expression(parser);
  if (!condition)
  {
    error(parser, "Expected condition in while statement");
    return NULL;
  }

  consume(parser, TOKEN_RPAREN, "Expected ')' after condition");

  ASTNode *body = parse_block(parser);
  if (!body)
  {
    free_ast(condition);
    return NULL;
  }

  return create_while_stmt(condition, body);
}

// --- For Loop ---

ASTNode *parse_for_stmt(Parser *parser)
{
  consume(parser, TOKEN_KEYWORD, "Expected 'for' keyword");
  consume(parser, TOKEN_LPAREN, "Expected '(' after 'for'");

  // Parse iterator variable
  if (!match(parser, TOKEN_IDENTIFIER))
  {
    error(parser, "Expected iterator variable name");
    return NULL;
  }
  char *iterator = strdup(previous(parser)->value);

  consume(parser, TOKEN_KEYWORD, "Expected 'in' keyword");
  if (strcmp(previous(parser)->value, "in") != 0)
  {
    error(parser, "Expected 'in' keyword");
    free(iterator);
    return NULL;
  }

  consume(parser, TOKEN_LBRACE, "Expected '{' after 'in'");

  // Parse range start
  ASTNode *start = parse_expression(parser);
  if (!start)
  {
    error(parser, "Expected range start expression");
    free(iterator);
    return NULL;
  }

  consume(parser, TOKEN_COLON, "Expected ':' in range");

  // Parse range end
  ASTNode *end = parse_expression(parser);
  if (!end)
  {
    error(parser, "Expected range end expression");
    free(iterator);
    free_ast(start);
    return NULL;
  }

  consume(parser, TOKEN_RBRACE, "Expected '}' after range");
  consume(parser, TOKEN_RPAREN, "Expected ')' after for loop header");

  // Parse loop body
  ASTNode *body = parse_block(parser);
  if (!body)
  {
    free(iterator);
    free_ast(start);
    free_ast(end);
    return NULL;
  }

  return create_for_stmt(iterator, start, end, body);
}

// --- Function Definition ---

ASTNode *parse_func_def(Parser *parser)
{
  // Check for comptime
  int is_comptime = 0;
  if (match(parser, TOKEN_KEYWORD) && strcmp(previous(parser)->value, "comptime") == 0)
  {
    is_comptime = 1;
  }

  // Parse fn keyword
  consume(parser, TOKEN_KEYWORD, "Expected 'fn' keyword");
  if (strcmp(previous(parser)->value, "fn") != 0)
  {
    error(parser, "Expected 'fn' keyword");
    return NULL;
  }

  // Parse function name
  if (!match(parser, TOKEN_IDENTIFIER))
  {
    error(parser, "Expected function name");
    return NULL;
  }
  char *name = strdup(previous(parser)->value);

  // Parse parameter list
  consume(parser, TOKEN_LPAREN, "Expected '(' after function name");

  ASTNode **parameters = NULL;
  int param_count = 0;

  if (!check(parser, TOKEN_RPAREN))
  {
    do
    {
      // Parse parameter name
      if (!match(parser, TOKEN_IDENTIFIER))
      {
        error(parser, "Expected parameter name");
        free(name);
        // Free already parsed parameters
        for (int i = 0; i < param_count; i++)
        {
          free_ast(parameters[i]);
        }
        free(parameters);
        return NULL;
      }
      char *param_name = strdup(previous(parser)->value);

      // Parse parameter type
      consume(parser, TOKEN_COLON, "Expected ':' after parameter name");
      if (!match(parser, TOKEN_KEYWORD))
      {
        error(parser, "Expected parameter type");
        free(name);
        free(param_name);
        for (int i = 0; i < param_count; i++)
        {
          free_ast(parameters[i]);
        }
        free(parameters);
        return NULL;
      }
      char *param_type = strdup(previous(parser)->value);

      // Create parameter node (as a var_decl)
      parameters = realloc(parameters, (param_count + 1) * sizeof(ASTNode *));
      parameters[param_count++] = create_var_decl(0, param_name, param_type, NULL);

      free(param_name);
      free(param_type);
    } while (match(parser, TOKEN_COMMA));
  }

  consume(parser, TOKEN_RPAREN, "Expected ')' after parameters");

  // Parse optional return type
  char *return_type = NULL;
  if (match(parser, TOKEN_COLON))
  {
    if (!match(parser, TOKEN_KEYWORD))
    {
      error(parser, "Expected return type after ':'");
      free(name);
      for (int i = 0; i < param_count; i++)
      {
        free_ast(parameters[i]);
      }
      free(parameters);
      return NULL;
    }
    return_type = strdup(previous(parser)->value);
  }

  // Parse function body
  ASTNode *body = parse_block(parser);
  if (!body)
  {
    free(name);
    free(return_type);
    for (int i = 0; i < param_count; i++)
    {
      free_ast(parameters[i]);
    }
    free(parameters);
    return NULL;
  }

  return create_func_def(name, parameters, param_count, return_type, body, is_comptime);
}

// --- Break Statement ---

ASTNode *parse_break_stmt(Parser *parser)
{
  if (peek(parser)->type != TOKEN_KEYWORD ||
      strcmp(peek(parser)->value, "break") != 0)
    error(parser, "Expected 'break' keyword");
  advance(parser); // consume "break"
  if (!match(parser, TOKEN_SEMICOLON))
    error(parser, "Expected ';' after 'break'");
  // Here we create a literal node as a placeholder for break.
  return create_literal("break");
}

// --- Continue Statement ---

ASTNode *parse_continue_stmt(Parser *parser)
{
  if (peek(parser)->type != TOKEN_KEYWORD ||
      strcmp(peek(parser)->value, "continue") != 0)
    error(parser, "Expected 'continue' keyword");
  advance(parser); // consume "continue"
  if (!match(parser, TOKEN_SEMICOLON))
    error(parser, "Expected ';' after 'continue'");
  // Here we create a literal node as a placeholder for continue.
  return create_literal("continue");
}

// --- Expression Parsing Functions ---

ASTNode *parse_expression(Parser *parser) { return parse_assignment(parser); }

// Parse a block: "{" { statement } "}"
ASTNode *parse_block(Parser *parser)
{
  consume(parser, TOKEN_LBRACE, "Expected '{' before block");

  ASTNode **statements = NULL;
  int stmt_count = 0;

  while (!check(parser, TOKEN_RBRACE) && !is_at_end(parser))
  {
    ASTNode *stmt = parse_statement(parser);
    if (stmt)
    {
      statements = realloc(statements, (stmt_count + 1) * sizeof(ASTNode *));
      statements[stmt_count++] = stmt;
    }
  }

  consume(parser, TOKEN_RBRACE, "Expected '}' after block");

  return create_block(statements, stmt_count);
}

// --- Expression Parsing End ---
