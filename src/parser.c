#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Utility Functions ---

Parser init_parser(TokenArray tokens)
{
  Parser parser;
  parser.tokens = tokens;
  parser.current = 0;
  return parser;
}

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
    else
      error(parser, "Unexpected keyword in statement");
  }
  else if (token->type == TOKEN_LBRACE)
  {
    return parse_block(parser);
  }
  return parse_expression_stmt(parser);
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
  // Expect "let"
  if (peek(parser)->type != TOKEN_KEYWORD ||
      strcmp(peek(parser)->value, "let") != 0)
    error(parser, "Expected 'let' in variable declaration");
  advance(parser); // consume "let"

  int is_const = 0;
  if (peek(parser)->type == TOKEN_KEYWORD &&
      strcmp(peek(parser)->value, "const") == 0)
  {
    is_const = 1;
    advance(parser); // consume "const"
  }

  if (peek(parser)->type != TOKEN_IDENTIFIER)
    error(parser, "Expected identifier in variable declaration");
  char *identifier = strdup(peek(parser)->value);
  advance(parser);

  char *type_annotation = NULL;
  if (match(parser, TOKEN_COLON))
  {
    if (peek(parser)->type != TOKEN_KEYWORD)
      error(parser, "Expected type keyword after ':'");
    type_annotation = strdup(peek(parser)->value);
    advance(parser);
  }

  // Expect '=' operator
  if (peek(parser)->type != TOKEN_OPERATOR ||
      strcmp(peek(parser)->value, "=") != 0)
    error(parser, "Expected '=' in variable declaration");
  advance(parser); // consume '='

  ASTNode *initializer = parse_expression(parser);

  if (!match(parser, TOKEN_SEMICOLON))
    error(parser, "Expected ';' after variable declaration");

  return create_var_decl(is_const, identifier, type_annotation, initializer);
}

// --- If Statement ---

ASTNode *parse_if_stmt(Parser *parser)
{
  if (peek(parser)->type != TOKEN_KEYWORD ||
      strcmp(peek(parser)->value, "if") != 0)
    error(parser, "Expected 'if' keyword");
  advance(parser); // consume "if"

  if (!match(parser, TOKEN_LPAREN))
    error(parser, "Expected '(' after 'if'");
  ASTNode *condition = parse_expression(parser);
  if (!match(parser, TOKEN_RPAREN))
    error(parser, "Expected ')' after if condition");

  ASTNode *if_block = parse_statement(parser);

  // Optional else branch
  ASTNode *else_block = NULL;
  if (peek(parser)->type == TOKEN_KEYWORD &&
      strcmp(peek(parser)->value, "else") == 0)
  {
    advance(parser); // consume "else"
    else_block = parse_statement(parser);
  }
  return create_if_stmt(condition, if_block, else_block);
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
  if (peek(parser)->type != TOKEN_KEYWORD ||
      strcmp(peek(parser)->value, "while") != 0)
    error(parser, "Expected 'while' keyword");
  advance(parser); // consume "while"
  if (!match(parser, TOKEN_LPAREN))
    error(parser, "Expected '(' after 'while'");
  ASTNode *condition = parse_expression(parser);
  if (!match(parser, TOKEN_RPAREN))
    error(parser, "Expected ')' after while condition");
  ASTNode *block = parse_statement(parser);
  return create_while_stmt(condition, block);
}

// --- For Loop ---

ASTNode *parse_for_stmt(Parser *parser)
{
  if (peek(parser)->type != TOKEN_KEYWORD ||
      strcmp(peek(parser)->value, "for") != 0)
    error(parser, "Expected 'for' keyword");
  advance(parser); // consume "for"
  if (!match(parser, TOKEN_LPAREN))
    error(parser, "Expected '(' after 'for'");

  if (peek(parser)->type != TOKEN_IDENTIFIER)
    error(parser, "Expected identifier in for loop");
  char *iterator = strdup(peek(parser)->value);
  advance(parser);

  if (!(peek(parser)->type == TOKEN_KEYWORD &&
        strcmp(peek(parser)->value, "in") == 0))
    error(parser, "Expected 'in' in for loop");
  advance(parser); // consume "in"

  if (!match(parser, TOKEN_LBRACE))
    error(parser, "Expected '{' in for loop range");

  ASTNode *start_expr = parse_expression(parser);
  if (!match(parser, TOKEN_COLON))
    error(parser, "Expected ':' in for loop range");
  ASTNode *end_expr = parse_expression(parser);
  if (!match(parser, TOKEN_RBRACE))
    error(parser, "Expected '}' in for loop range");
  if (!match(parser, TOKEN_RPAREN))
    error(parser, "Expected ')' after for loop range");

  ASTNode *block = parse_statement(parser);
  return create_for_stmt(iterator, start_expr, end_expr, block);
}

// --- Function Definition ---

ASTNode *parse_func_def(Parser *parser)
{
  // Optionally consume "comptime"
  if (peek(parser)->type == TOKEN_KEYWORD &&
      strcmp(peek(parser)->value, "comptime") == 0)
    advance(parser);

  if (peek(parser)->type != TOKEN_KEYWORD ||
      strcmp(peek(parser)->value, "fn") != 0)
    error(parser, "Expected 'fn' keyword for function definition");
  advance(parser); // consume "fn"

  if (peek(parser)->type != TOKEN_IDENTIFIER)
    error(parser, "Expected function name");
  char *name = strdup(peek(parser)->value);
  advance(parser);

  if (!match(parser, TOKEN_LPAREN))
    error(parser, "Expected '(' after function name");

  // Parse parameter list
  ASTNode **parameters = NULL;
  int param_count = 0;
  int param_capacity = 4;
  parameters = malloc(param_capacity * sizeof(ASTNode *));
  if (!parameters)
    error(parser, "Memory allocation failed for function parameters");

  // Parse parameters until we find a closing parenthesis
  if (peek(parser)->type != TOKEN_RPAREN)
  {
    do
    {
      if (peek(parser)->type != TOKEN_IDENTIFIER)
        error(parser, "Expected parameter name");
      char *param_name = strdup(peek(parser)->value);
      advance(parser);

      if (!match(parser, TOKEN_COLON))
        error(parser, "Expected ':' after parameter name");

      if (peek(parser)->type != TOKEN_KEYWORD)
        error(parser, "Expected parameter type");
      char *param_type = strdup(peek(parser)->value);
      advance(parser);

      // Create parameter node
      ASTNode *param = create_var_decl(0, param_name, param_type, NULL);
      free(param_name);
      free(param_type);

      // Add parameter to array
      if (param_count >= param_capacity)
      {
        param_capacity *= 2;
        ASTNode **temp = realloc(parameters, param_capacity * sizeof(ASTNode *));
        if (!temp)
        {
          free(parameters);
          error(parser, "Memory reallocation failed for function parameters");
        }
        parameters = temp;
      }
      parameters[param_count++] = param;

      // Check for comma
      if (peek(parser)->type == TOKEN_OPERATOR &&
          strcmp(peek(parser)->value, ",") == 0)
      {
        advance(parser);
      }
      else
      {
        break;
      }
    } while (1);
  }

  if (!match(parser, TOKEN_RPAREN))
    error(parser, "Expected ')' after parameter list");

  char *return_type = NULL;
  if (match(parser, TOKEN_COLON))
  {
    if (peek(parser)->type != TOKEN_KEYWORD)
      error(parser, "Expected return type after ':'");
    return_type = strdup(peek(parser)->value);
    advance(parser);
  }
  ASTNode *body = parse_statement(parser);
  return create_func_def(name, parameters, param_count, return_type, body);
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

ASTNode *parse_assignment(Parser *parser)
{
  ASTNode *left = parse_logical_or(parser);
  if (peek(parser)->type == TOKEN_OPERATOR &&
      strcmp(peek(parser)->value, "=") == 0)
  {
    advance(parser); // consume "="
    ASTNode *right = parse_assignment(parser);
    return create_assign_expr(left, right);
  }
  return left;
}

ASTNode *parse_logical_or(Parser *parser)
{
  ASTNode *left = parse_logical_and(parser);
  while (peek(parser)->type == TOKEN_KEYWORD &&
         strcmp(peek(parser)->value, "or") == 0)
  {
    char *op = strdup("or");
    advance(parser);
    ASTNode *right = parse_logical_and(parser);
    left = create_binary_expr(op, left, right);
  }
  return left;
}

ASTNode *parse_logical_and(Parser *parser)
{
  ASTNode *left = parse_equality(parser);
  while (peek(parser)->type == TOKEN_KEYWORD &&
         strcmp(peek(parser)->value, "and") == 0)
  {
    char *op = strdup("and");
    advance(parser);
    ASTNode *right = parse_equality(parser);
    left = create_binary_expr(op, left, right);
  }
  return left;
}

ASTNode *parse_equality(Parser *parser)
{
  ASTNode *left = parse_relational(parser);
  while (peek(parser)->type == TOKEN_OPERATOR &&
         (strcmp(peek(parser)->value, "==") == 0 ||
          strcmp(peek(parser)->value, "!=") == 0))
  {
    char *op = strdup(peek(parser)->value);
    advance(parser);
    ASTNode *right = parse_relational(parser);
    left = create_binary_expr(op, left, right);
  }
  return left;
}

ASTNode *parse_relational(Parser *parser)
{
  ASTNode *left = parse_additive(parser);
  while (peek(parser)->type == TOKEN_OPERATOR &&
         (strcmp(peek(parser)->value, "<") == 0 ||
          strcmp(peek(parser)->value, ">") == 0 ||
          strcmp(peek(parser)->value, "<=") == 0 ||
          strcmp(peek(parser)->value, ">=") == 0))
  {
    char *op = strdup(peek(parser)->value);
    advance(parser);
    ASTNode *right = parse_additive(parser);
    left = create_binary_expr(op, left, right);
  }
  return left;
}

ASTNode *parse_additive(Parser *parser)
{
  ASTNode *left = parse_multiplicative(parser);
  while (peek(parser)->type == TOKEN_OPERATOR &&
         (strcmp(peek(parser)->value, "+") == 0 ||
          strcmp(peek(parser)->value, "-") == 0))
  {
    char *op = strdup(peek(parser)->value);
    advance(parser);
    ASTNode *right = parse_multiplicative(parser);
    left = create_binary_expr(op, left, right);
  }
  return left;
}

ASTNode *parse_multiplicative(Parser *parser)
{
  ASTNode *left = parse_unary(parser);
  while (peek(parser)->type == TOKEN_OPERATOR &&
         (strcmp(peek(parser)->value, "*") == 0 ||
          strcmp(peek(parser)->value, "/") == 0 ||
          strcmp(peek(parser)->value, "%") == 0))
  {
    char *op = strdup(peek(parser)->value);
    advance(parser);
    ASTNode *right = parse_unary(parser);
    left = create_binary_expr(op, left, right);
  }
  return left;
}

ASTNode *parse_unary(Parser *parser)
{
  Token *token = peek(parser);
  if ((token->type == TOKEN_OPERATOR &&
       (strcmp(token->value, "-") == 0 || strcmp(token->value, "+") == 0)) ||
      (token->type == TOKEN_KEYWORD && strcmp(token->value, "not") == 0))
  {
    char *op = strdup(token->value);
    advance(parser);
    ASTNode *operand = parse_unary(parser);
    return create_unary_expr(op, operand);
  }
  return parse_primary(parser);
}

ASTNode *parse_primary(Parser *parser)
{
  Token *token = peek(parser);
  if (token->type == TOKEN_IDENTIFIER)
  {
    char *name = strdup(token->value);
    advance(parser);
    // If next token is "(", parse as function call.
    if (peek(parser)->type == TOKEN_LPAREN)
    {
      advance(parser); // consume "("
      ASTNode **arguments = NULL;
      int arg_count = 0;
      int arg_capacity = 4;
      arguments = malloc(arg_capacity * sizeof(ASTNode *));
      if (!arguments)
        error(parser, "Memory allocation failed for function call arguments");
      // Parse arguments until ")".
      if (peek(parser)->type != TOKEN_RPAREN)
      {
        do
        {
          ASTNode *arg = parse_expression(parser);
          if (!arg)
            break;
          if (arg_count >= arg_capacity)
          {
            arg_capacity *= 2;
            ASTNode **temp =
                realloc(arguments, arg_capacity * sizeof(ASTNode *));
            if (!temp)
            {
              free(arguments);
              error(parser,
                    "Memory reallocation failed for function call arguments");
            }
            arguments = temp;
          }
          arguments[arg_count++] = arg;
          if (peek(parser)->type == TOKEN_OPERATOR &&
              strcmp(peek(parser)->value, ",") == 0)
            advance(parser);
          else
            break;
        } while (1);
      }
      if (!match(parser, TOKEN_RPAREN))
        error(parser, "Expected ')' after function call arguments");
      return create_func_call(name, arguments, arg_count);
    }
    return create_identifier(name);
  }
  else if (token->type == TOKEN_INTEGER || token->type == TOKEN_FLOAT)
  {
    char *value = strdup(token->value);
    advance(parser);
    return create_literal(value);
  }
  else if (token->type == TOKEN_STRING || token->type == TOKEN_FSTRING)
  {
    char *value = strdup(token->value);
    advance(parser);
    return create_literal(value);
  }
  else if (token->type == TOKEN_KEYWORD &&
           (strcmp(token->value, "true") == 0 || strcmp(token->value, "false") == 0))
  {
    char *value = strdup(token->value);
    advance(parser);
    return create_literal(value);
  }
  else if (token->type == TOKEN_LPAREN)
  {
    advance(parser); // consume "("
    ASTNode *expr = parse_expression(parser);
    if (!match(parser, TOKEN_RPAREN))
      error(parser, "Expected ')' after expression");
    return expr;
  }
  else if (token->type == TOKEN_LBRACE)
  {
    return parse_block(parser);
  }
  else
  {
    error(parser, "Unexpected token in expression");
    return NULL;
  }
}

// Parse a block: "{" { statement } "}"
ASTNode *parse_block(Parser *parser)
{
  if (!match(parser, TOKEN_LBRACE))
    error(parser, "Expected '{' at start of block");

  int capacity = 4;
  int count = 0;
  ASTNode **statements = malloc(capacity * sizeof(ASTNode *));
  if (!statements)
    error(parser, "Memory allocation failed for block statements");

  while (peek(parser)->type != TOKEN_RBRACE &&
         peek(parser)->type != TOKEN_EOF)
  {
    ASTNode *stmt;
    // If we're at the last statement and there's no semicolon, treat it as a return expression
    Token *next = peek(parser);
    int is_last_expr = 0;

    // Look ahead to see if this is the last expression in the block
    if (next->type != TOKEN_KEYWORD && next->type != TOKEN_LBRACE)
    {
      Token *ahead = &parser->tokens.tokens[parser->current + 1];
      if (ahead->type == TOKEN_RBRACE)
      {
        is_last_expr = 1;
        stmt = create_expr_stmt(parse_expression(parser));
      }
      else
      {
        stmt = parse_statement(parser);
      }
    }
    else
    {
      stmt = parse_statement(parser);
    }

    if (!stmt)
      break;

    if (count >= capacity)
    {
      capacity *= 2;
      ASTNode **temp = realloc(statements, capacity * sizeof(ASTNode *));
      if (!temp)
      {
        free(statements);
        error(parser, "Memory reallocation failed for block statements");
      }
      statements = temp;
    }
    statements[count++] = stmt;
  }
  if (!match(parser, TOKEN_RBRACE))
    error(parser, "Expected '}' at end of block");

  return create_block(statements, count);
}

// --- Expression Parsing End ---
