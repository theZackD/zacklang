#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "ast.h"

#define DEBUG_PRINT(...) printf(__VA_ARGS__)

// Forward declarations for internal functions
static Token peek(Parser *parser);
static Token previous(Parser *parser);
static int is_at_end(Parser *parser);
static Token advance(Parser *parser);
static int check(Parser *parser, TokenType type);
static int match(Parser *parser, TokenType type);
static void consume(Parser *parser, TokenType type, const char *message);
static ASTNode *parse_term(Parser *parser);
static ASTNode *parse_factor(Parser *parser);
static ASTNode *parse_unary(Parser *parser);
static ASTNode *parse_primary(Parser *parser);

// Helper functions for token handling
static Token peek(Parser *parser)
{
    Token t = parser->tokens.tokens[parser->current];
    printf("DEBUG: peek() at position %d: type=%d value='%s'\n",
           parser->current, t.type, t.value);
    return t;
}

static Token previous(Parser *parser)
{
    Token t = parser->tokens.tokens[parser->current - 1];
    printf("DEBUG: previous() at position %d: type=%d value='%s'\n",
           parser->current - 1, t.type, t.value);
    return t;
}

static int is_at_end(Parser *parser)
{
    return peek(parser).type == TOKEN_EOF;
}

static Token advance(Parser *parser)
{
    printf("DEBUG: advance() from position %d\n", parser->current);
    if (!is_at_end(parser))
    {
        parser->current++;
    }
    return previous(parser);
}

static int check(Parser *parser, TokenType type)
{
    if (is_at_end(parser))
        return 0;
    return peek(parser).type == type;
}

static int match(Parser *parser, TokenType type)
{
    printf("DEBUG: match() checking for type %d, current token type=%d value='%s'\n",
           type, peek(parser).type, peek(parser).value);
    if (check(parser, type))
    {
        advance(parser);
        printf("DEBUG: match() found match, advanced to type=%d value='%s'\n",
               peek(parser).type, peek(parser).value);
        return 1;
    }
    return 0;
}

static void consume(Parser *parser, TokenType type, const char *message)
{
    if (check(parser, type))
    {
        advance(parser);
        return;
    }

    parser_error(parser, message);
}

// Create a new parser instance
Parser *create_parser(TokenArray tokens)
{
    Parser *parser = malloc(sizeof(Parser));
    parser->tokens = tokens;
    parser->current = 0;
    parser->had_error = 0;
    return parser;
}

// Free parser resources
void destroy_parser(Parser *parser)
{
    // Note: We don't free tokens here as they're owned by the caller
    free(parser);
}

// Error reporting
void parser_error(Parser *parser, const char *message)
{
    Token token = peek(parser);
    fprintf(stderr, "[line %d] Error at '%s': %s\n",
            token.line, token.value, message);
    parser->had_error = 1;
}

// Forward declarations for recursive descent
static ASTNode *parse_primary(Parser *parser);
static ASTNode *parse_unary(Parser *parser);
static ASTNode *parse_power(Parser *parser);
static ASTNode *parse_multiplicative(Parser *parser);
static ASTNode *parse_additive(Parser *parser);
static ASTNode *parse_relational(Parser *parser);
static ASTNode *parse_equality(Parser *parser);
static ASTNode *parse_logical_and(Parser *parser);
static ASTNode *parse_logical_or(Parser *parser);
static ASTNode *parse_assignment(Parser *parser);

// Parse primary expressions (literals, identifiers, parenthesized expressions)
static ASTNode *parse_primary(Parser *parser)
{
    DEBUG_PRINT("Parsing primary expression\n");
    DEBUG_PRINT("Current token: type=%d, value='%s'\n", peek(parser).type, peek(parser).value);

    if (match(parser, TOKEN_INTEGER))
    {
        DEBUG_PRINT("Found literal: %s\n", previous(parser).value);
        return create_literal(previous(parser).value);
    }

    if (match(parser, TOKEN_FLOAT))
    {
        DEBUG_PRINT("Found literal: %s\n", previous(parser).value);
        return create_literal(previous(parser).value);
    }

    if (match(parser, TOKEN_STRING))
    {
        DEBUG_PRINT("Found literal: %s\n", previous(parser).value);
        return create_literal(previous(parser).value);
    }

    if (match(parser, TOKEN_IDENTIFIER))
    {
        DEBUG_PRINT("Found identifier: %s\n", previous(parser).value);
        return create_identifier(previous(parser).value);
    }

    if (match(parser, TOKEN_KEYWORD))
    {
        const char *value = previous(parser).value;
        DEBUG_PRINT("Found keyword: %s\n", value);
        if (strcmp(value, "true") == 0 || strcmp(value, "false") == 0)
        {
            return create_literal(value);
        }
        parser->current--; // Rewind if not a boolean keyword
    }

    if (match(parser, TOKEN_LPAREN))
    {
        ASTNode *expr = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
        return expr;
    }

    parser_error(parser, "Expected expression");
    return NULL;
}

// Parse unary expressions (-, +, not)
static ASTNode *parse_unary(Parser *parser)
{
    DEBUG_PRINT("Parsing unary expression\n");

    if (match(parser, TOKEN_OPERATOR))
    {
        const char *op = previous(parser).value;
        if (strcmp(op, "-") == 0 || strcmp(op, "+") == 0)
        {
            DEBUG_PRINT("Found unary operator: %s\n", op);
            ASTNode *right = parse_unary(parser);
            if (right == NULL)
                return NULL;
            return create_unary_expr((char *)op, right);
        }
        parser->current--; // Rewind if not a unary operator
    }
    else if (match(parser, TOKEN_KEYWORD))
    {
        const char *keyword = previous(parser).value;
        if (strcmp(keyword, "not") == 0)
        {
            DEBUG_PRINT("Found unary operator: not\n");
            ASTNode *right = parse_unary(parser);
            if (right == NULL)
                return NULL;
            return create_unary_expr((char *)"not", right);
        }
        parser->current--; // Rewind if not a unary operator keyword
    }

    return parse_primary(parser);
}

// Parse power expressions (right-associative)
static ASTNode *parse_power(Parser *parser)
{
    ASTNode *left = parse_unary(parser);
    if (!left)
        return NULL;

    if (check(parser, TOKEN_OPERATOR) && strcmp(peek(parser).value, "**") == 0)
    {
        advance(parser); // Consume the operator
        const char *op = previous(parser).value;
        ASTNode *right = parse_power(parser); // Right-associative
        if (!right)
        {
            free_ast(left);
            return NULL;
        }
        left = create_binary_expr((char *)op, left, right);
    }

    return left;
}

// Parse multiplicative expressions (*, /, %)
static ASTNode *parse_multiplicative(Parser *parser)
{
    ASTNode *left = parse_power(parser);
    if (!left)
        return NULL;

    while (check(parser, TOKEN_OPERATOR))
    {
        const char *op = peek(parser).value;
        if (strcmp(op, "*") != 0 && strcmp(op, "/") != 0 && strcmp(op, "%") != 0)
            break;

        advance(parser); // Consume the operator
        ASTNode *right = parse_power(parser);
        if (!right)
        {
            free_ast(left);
            return NULL;
        }
        left = create_binary_expr((char *)op, left, right);
    }

    return left;
}

// Parse additive expressions (+, -)
static ASTNode *parse_additive(Parser *parser)
{
    ASTNode *left = parse_multiplicative(parser);
    if (!left)
        return NULL;

    while (check(parser, TOKEN_OPERATOR))
    {
        const char *op = peek(parser).value;
        if (strcmp(op, "+") != 0 && strcmp(op, "-") != 0)
            break;

        advance(parser); // Consume the operator
        ASTNode *right = parse_multiplicative(parser);
        if (!right)
        {
            free_ast(left);
            return NULL;
        }
        left = create_binary_expr((char *)op, left, right);
    }

    return left;
}

// Parse relational expressions (<, >, <=, >=)
static ASTNode *parse_relational(Parser *parser)
{
    ASTNode *left = parse_additive(parser);
    if (!left)
        return NULL;

    while (match(parser, TOKEN_OPERATOR))
    {
        const char *op = previous(parser).value;
        if (strcmp(op, "<") != 0 && strcmp(op, ">") != 0 &&
            strcmp(op, "<=") != 0 && strcmp(op, ">=") != 0)
        {
            parser->current--;
            break;
        }

        ASTNode *right = parse_additive(parser);
        if (!right)
        {
            free_ast(left);
            return NULL;
        }
        left = create_binary_expr((char *)op, left, right);
    }

    return left;
}

// Parse equality expressions (==, !=)
static ASTNode *parse_equality(Parser *parser)
{
    ASTNode *left = parse_relational(parser);
    if (!left)
        return NULL;

    while (match(parser, TOKEN_OPERATOR))
    {
        const char *op = previous(parser).value;
        if (strcmp(op, "==") != 0 && strcmp(op, "!=") != 0)
        {
            parser->current--;
            break;
        }

        ASTNode *right = parse_relational(parser);
        if (!right)
        {
            free_ast(left);
            return NULL;
        }
        left = create_binary_expr((char *)op, left, right);
    }

    return left;
}

// Parse logical AND expressions
static ASTNode *parse_logical_and(Parser *parser)
{
    ASTNode *left = parse_equality(parser);
    if (!left)
        return NULL;

    while (check(parser, TOKEN_KEYWORD) && strcmp(peek(parser).value, "and") == 0)
    {
        advance(parser); // Consume the operator
        const char *op = previous(parser).value;
        ASTNode *right = parse_equality(parser);
        if (!right)
        {
            free_ast(left);
            return NULL;
        }
        left = create_binary_expr((char *)op, left, right);
    }

    return left;
}

// Parse logical OR expressions
static ASTNode *parse_logical_or(Parser *parser)
{
    ASTNode *left = parse_logical_and(parser);
    if (!left)
        return NULL;

    while (check(parser, TOKEN_KEYWORD) && strcmp(peek(parser).value, "or") == 0)
    {
        advance(parser); // Consume the operator
        const char *op = previous(parser).value;
        ASTNode *right = parse_logical_and(parser);
        if (!right)
        {
            free_ast(left);
            return NULL;
        }
        left = create_binary_expr((char *)op, left, right);
    }

    return left;
}

// Parse assignment expressions
static ASTNode *parse_assignment(Parser *parser)
{
    ASTNode *left = parse_logical_or(parser);
    if (!left)
        return NULL;

    if (match(parser, TOKEN_OPERATOR) && strcmp(previous(parser).value, "=") == 0)
    {
        // Ensure left side is a valid assignment target
        if (left->type != AST_IDENTIFIER)
        {
            parser_error(parser, "Invalid assignment target");
            free_ast(left);
            return NULL;
        }

        ASTNode *right = parse_assignment(parser);
        if (!right)
        {
            free_ast(left);
            return NULL;
        }
        return create_assign_expr(left, right);
    }

    return left;
}

// Main expression parsing function
ASTNode *parse_expression(Parser *parser)
{
    DEBUG_PRINT("Parsing expression\n");
    return parse_assignment(parser);
}

// Parse a variable declaration
// let [const] identifier [:type] = expression;
ASTNode *parse_var_declaration(Parser *parser)
{
    // Expect 'let' keyword
    if (!match(parser, TOKEN_KEYWORD) || strcmp(previous(parser).value, "let") != 0)
    {
        parser_error(parser, "Expected 'let' keyword");
        return NULL;
    }

    // Check for const modifier
    int is_const = 0;
    if (match(parser, TOKEN_KEYWORD) && strcmp(previous(parser).value, "const") == 0)
    {
        is_const = 1;
    }

    // Expect identifier
    if (!match(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, "Expected variable name");
        return NULL;
    }
    char *identifier = strdup(previous(parser).value);

    // Check for type annotation
    char *type_annotation = NULL;
    if (match(parser, TOKEN_COLON))
    {
        if (!match(parser, TOKEN_KEYWORD))
        {
            parser_error(parser, "Expected type after ':'");
            free(identifier);
            return NULL;
        }
        type_annotation = strdup(previous(parser).value);
    }

    // Expect '='
    if (!match(parser, TOKEN_OPERATOR) || strcmp(previous(parser).value, "=") != 0)
    {
        parser_error(parser, "Expected '=' after variable declaration");
        free(identifier);
        free(type_annotation);
        return NULL;
    }

    // Parse initializer expression
    ASTNode *initializer = parse_expression(parser);
    if (!initializer)
    {
        free(identifier);
        free(type_annotation);
        return NULL;
    }

    // Expect semicolon
    if (!match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Expected ';' after variable declaration");
        free(identifier);
        free(type_annotation);
        free_ast(initializer);
        return NULL;
    }

    return create_var_decl(is_const, identifier, type_annotation, initializer);
}

// Parse a block of statements
ASTNode *parse_block(Parser *parser)
{
    // Expect opening brace
    if (!match(parser, TOKEN_LBRACE))
    {
        parser_error(parser, "Expected '{' before block");
        return NULL;
    }

    // Parse statements until we hit a closing brace
    ASTNode **statements = NULL;
    int stmt_count = 0;
    int capacity = 8; // Initial capacity
    statements = malloc(capacity * sizeof(ASTNode *));

    while (!check(parser, TOKEN_RBRACE) && !is_at_end(parser))
    {
        // For now, only handle variable declarations
        ASTNode *stmt = parse_var_declaration(parser);
        if (!stmt)
        {
            // Free already parsed statements
            for (int i = 0; i < stmt_count; i++)
            {
                free_ast(statements[i]);
            }
            free(statements);
            return NULL;
        }

        // Resize array if needed
        if (stmt_count >= capacity)
        {
            capacity *= 2;
            statements = realloc(statements, capacity * sizeof(ASTNode *));
        }

        statements[stmt_count++] = stmt;
    }

    // Expect closing brace
    if (!match(parser, TOKEN_RBRACE))
    {
        parser_error(parser, "Expected '}' after block");
        // Free statements
        for (int i = 0; i < stmt_count; i++)
        {
            free_ast(statements[i]);
        }
        free(statements);
        return NULL;
    }

    return create_block(statements, stmt_count);
}

// Parse a function declaration
// [comptime] fn identifier(params) [:type] { body }
ASTNode *parse_function_declaration(Parser *parser)
{
    printf("DEBUG: Starting function declaration parse\n");
    // Check for comptime modifier
    int is_comptime = 0;
    if (match(parser, TOKEN_KEYWORD))
    {
        printf("DEBUG: Found keyword: %s\n", previous(parser).value);
        if (strcmp(previous(parser).value, "comptime") == 0)
        {
            is_comptime = 1;
        }
        else
        {
            // If it's not comptime, we need to rewind because it might be 'fn'
            parser->current--;
        }
    }

    // Expect 'fn' keyword
    if (!match(parser, TOKEN_KEYWORD) || strcmp(previous(parser).value, "fn") != 0)
    {
        printf("DEBUG: Expected 'fn', got token type %d with value '%s'\n",
               peek(parser).type, peek(parser).value);
        parser_error(parser, "Expected 'fn' keyword");
        return NULL;
    }

    printf("DEBUG: Found fn keyword\n");

    // Expect function name
    if (!match(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, "Expected function name");
        return NULL;
    }
    char *name = strdup(previous(parser).value);
    printf("DEBUG: Found function name: %s\n", name);

    // Expect opening parenthesis
    if (!match(parser, TOKEN_LPAREN))
    {
        parser_error(parser, "Expected '(' after function name");
        free(name);
        return NULL;
    }

    // Parse parameters
    ASTNode **parameters = NULL;
    int param_count = 0;
    if (!check(parser, TOKEN_RPAREN))
    {
        parameters = malloc(sizeof(ASTNode *) * 8); // Initial capacity
        int capacity = 8;

        do
        {
            if (param_count >= capacity)
            {
                capacity *= 2;
                parameters = realloc(parameters, sizeof(ASTNode *) * capacity);
            }

            // Parse parameter: identifier [:type]
            if (!match(parser, TOKEN_IDENTIFIER))
            {
                parser_error(parser, "Expected parameter name");
                goto error;
            }
            char *param_name = strdup(previous(parser).value);

            // Check for type annotation
            char *param_type = NULL;
            if (match(parser, TOKEN_COLON))
            {
                if (!match(parser, TOKEN_KEYWORD))
                {
                    parser_error(parser, "Expected type after ':'");
                    free(param_name);
                    goto error;
                }
                param_type = strdup(previous(parser).value);
            }

            // Create parameter node (as a variable declaration)
            parameters[param_count++] = create_var_decl(0, param_name, param_type, NULL);
            free(param_name);
            if (param_type)
                free(param_type);

        } while (match(parser, TOKEN_COMMA));
    }

    // Expect closing parenthesis
    if (!match(parser, TOKEN_RPAREN))
    {
        parser_error(parser, "Expected ')' after parameters");
        goto error;
    }

    // Check for return type
    char *return_type = NULL;
    if (match(parser, TOKEN_COLON))
    {
        if (!match(parser, TOKEN_KEYWORD))
        {
            parser_error(parser, "Expected return type after ':'");
            goto error;
        }
        return_type = strdup(previous(parser).value);
    }

    // Parse function body
    ASTNode *body = parse_block(parser);
    if (!body)
    {
        if (return_type)
            free(return_type);
        goto error;
    }

    return create_func_def(name, parameters, param_count, return_type, body, is_comptime);

error:
    free(name);
    if (parameters)
    {
        for (int i = 0; i < param_count; i++)
        {
            free_ast(parameters[i]);
        }
        free(parameters);
    }
    return NULL;
}

static ASTNode *parse_factor(Parser *parser)
{
    DEBUG_PRINT("Parsing factor\n");
    ASTNode *left = parse_unary(parser);
    if (left == NULL)
        return NULL;

    // Use check/peek approach for multiplicative operators
    while (check(parser, TOKEN_OPERATOR))
    {
        Token opToken = peek(parser);
        // Only handle multiplicative operators
        if (strcmp(opToken.value, "*") != 0 && strcmp(opToken.value, "/") != 0 && strcmp(opToken.value, "%") != 0)
            break;
        // Consume the operator
        advance(parser);
        DEBUG_PRINT("Found binary operator: %s\n", opToken.value);
        ASTNode *right = parse_unary(parser);
        if (right == NULL)
        {
            free_ast(left);
            return NULL;
        }
        // Create a binary expression node
        left = create_binary_expr((char *)opToken.value, left, right);
    }

    return left;
}

static ASTNode *parse_term(Parser *parser)
{
    DEBUG_PRINT("Parsing term\n");
    ASTNode *left = parse_factor(parser);
    if (left == NULL)
        return NULL;

    // Instead of match() and then rewinding, use a check/peek approach
    while (check(parser, TOKEN_OPERATOR))
    {
        Token opToken = peek(parser);
        // Only handle additive operators
        if (strcmp(opToken.value, "+") != 0 && strcmp(opToken.value, "-") != 0)
            break;
        // Consume the operator
        advance(parser);
        DEBUG_PRINT("Found binary operator: %s\n", opToken.value);
        ASTNode *right = parse_factor(parser);
        if (right == NULL)
        {
            free_ast(left);
            return NULL;
        }
        // Create a binary expression node
        left = create_binary_expr((char *)opToken.value, left, right);
    }

    return left;
}