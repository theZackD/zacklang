#include "../../include/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Helper function to verify token properties
void verify_token(Token token, TokenType expected_type, const char *expected_value, int expected_line, int expected_col)
{
    printf("Checking token: type=%d, value='%s', line=%d, col=%d (expected: type=%d, value='%s', line=%d, col=%d)\n",
           token.type, token.value, token.line, token.column,
           expected_type, expected_value, expected_line, expected_col);
    assert(token.type == expected_type);
    assert(strcmp(token.value, expected_value) == 0);
    assert(token.line == expected_line);
    assert(token.column == expected_col);
}

// Test single line token positions
void test_single_line(void)
{
    const char *input = "let x = 42;";
    TokenArray tokens = tokenize(input);

    verify_token(tokens.tokens[0], TOKEN_KEYWORD, "let", 1, 1);
    verify_token(tokens.tokens[1], TOKEN_IDENTIFIER, "x", 1, 5);
    verify_token(tokens.tokens[2], TOKEN_OPERATOR, "=", 1, 7);
    verify_token(tokens.tokens[3], TOKEN_INTEGER, "42", 1, 9);
    verify_token(tokens.tokens[4], TOKEN_SEMICOLON, ";", 1, 11);

    printf("✓ Single line position tracking test passed\n");
    free_token_array(&tokens);
}

// Test multi-line token positions
void test_multi_line(void)
{
    const char *input = "let x = 42;\nlet y = 10;";
    TokenArray tokens = tokenize(input);

    verify_token(tokens.tokens[0], TOKEN_KEYWORD, "let", 1, 1);
    verify_token(tokens.tokens[1], TOKEN_IDENTIFIER, "x", 1, 5);
    verify_token(tokens.tokens[2], TOKEN_OPERATOR, "=", 1, 7);
    verify_token(tokens.tokens[3], TOKEN_INTEGER, "42", 1, 9);
    verify_token(tokens.tokens[4], TOKEN_SEMICOLON, ";", 1, 11);

    verify_token(tokens.tokens[5], TOKEN_KEYWORD, "let", 2, 1);
    verify_token(tokens.tokens[6], TOKEN_IDENTIFIER, "y", 2, 5);
    verify_token(tokens.tokens[7], TOKEN_OPERATOR, "=", 2, 7);
    verify_token(tokens.tokens[8], TOKEN_INTEGER, "10", 2, 9);
    verify_token(tokens.tokens[9], TOKEN_SEMICOLON, ";", 2, 11);

    printf("✓ Multi-line position tracking test passed\n");
    free_token_array(&tokens);
}

// Test string literals
void test_string_positions(void)
{
    const char *input = "let msg = \"Hello, World!\";";
    TokenArray tokens = tokenize(input);

    verify_token(tokens.tokens[0], TOKEN_KEYWORD, "let", 1, 1);
    verify_token(tokens.tokens[1], TOKEN_IDENTIFIER, "msg", 1, 5);
    verify_token(tokens.tokens[2], TOKEN_OPERATOR, "=", 1, 9);
    verify_token(tokens.tokens[3], TOKEN_STRING, "Hello, World!", 1, 11);
    verify_token(tokens.tokens[4], TOKEN_SEMICOLON, ";", 1, 26);

    printf("✓ String literal position tracking test passed\n");
    free_token_array(&tokens);
}

// Test basic f-string with single interpolation
void test_basic_fstring(void)
{
    const char *input = "let msg = f\"Value is {x + 1}\";";
    TokenArray tokens = tokenize(input);

    verify_token(tokens.tokens[0], TOKEN_KEYWORD, "let", 1, 1);
    verify_token(tokens.tokens[1], TOKEN_IDENTIFIER, "msg", 1, 5);
    verify_token(tokens.tokens[2], TOKEN_OPERATOR, "=", 1, 9);
    verify_token(tokens.tokens[3], TOKEN_FSTRING, "Value is ", 1, 13);
    verify_token(tokens.tokens[4], TOKEN_IDENTIFIER, "x", 1, 21);
    verify_token(tokens.tokens[5], TOKEN_OPERATOR, "+", 1, 23);
    verify_token(tokens.tokens[6], TOKEN_INTEGER, "1", 1, 25);
    verify_token(tokens.tokens[7], TOKEN_SEMICOLON, ";", 1, 28);

    printf("✓ Basic f-string test passed\n");
    free_token_array(&tokens);
}

// Test f-string with numeric interpolation
void test_numeric_fstring(void)
{
    const char *input = "f\"Count: {42}\";";
    TokenArray tokens = tokenize(input);

    verify_token(tokens.tokens[0], TOKEN_FSTRING, "Count: ", 1, 3);
    verify_token(tokens.tokens[1], TOKEN_INTEGER, "42", 1, 9);
    verify_token(tokens.tokens[2], TOKEN_SEMICOLON, ";", 1, 13);

    printf("✓ Numeric f-string test passed\n");
    free_token_array(&tokens);
}

// Test f-string with identifier interpolation
void test_identifier_fstring(void)
{
    const char *input = "f\"Name: {name}\";";
    TokenArray tokens = tokenize(input);

    verify_token(tokens.tokens[0], TOKEN_FSTRING, "Name: ", 1, 3);
    verify_token(tokens.tokens[1], TOKEN_IDENTIFIER, "name", 1, 8);
    verify_token(tokens.tokens[2], TOKEN_SEMICOLON, ";", 1, 14);

    printf("✓ Identifier f-string test passed\n");
    free_token_array(&tokens);
}

// Test multi-line f-string
void test_multiline_fstring(void)
{
    const char *input = "let msg = f\"First line\nValue: {x}\";\n";
    TokenArray tokens = tokenize(input);

    verify_token(tokens.tokens[0], TOKEN_KEYWORD, "let", 1, 1);
    verify_token(tokens.tokens[1], TOKEN_IDENTIFIER, "msg", 1, 5);
    verify_token(tokens.tokens[2], TOKEN_OPERATOR, "=", 1, 9);
    verify_token(tokens.tokens[3], TOKEN_FSTRING, "First line\nValue: ", 2, 13);
    verify_token(tokens.tokens[4], TOKEN_IDENTIFIER, "x", 2, 8);
    verify_token(tokens.tokens[5], TOKEN_SEMICOLON, ";", 2, 10);

    printf("✓ Multi-line f-string test passed\n");
    free_token_array(&tokens);
}

// Test single line comments
void test_single_line_comments(void)
{
    // Test comment at the end of a line
    const char *input1 = "let x = 42; // This is a comment\nlet y = 10;";
    TokenArray tokens = tokenize(input1);

    verify_token(tokens.tokens[0], TOKEN_KEYWORD, "let", 1, 1);
    verify_token(tokens.tokens[1], TOKEN_IDENTIFIER, "x", 1, 5);
    verify_token(tokens.tokens[2], TOKEN_OPERATOR, "=", 1, 7);
    verify_token(tokens.tokens[3], TOKEN_INTEGER, "42", 1, 9);
    verify_token(tokens.tokens[4], TOKEN_SEMICOLON, ";", 1, 11);
    // Comment should be ignored, next token should be on next line
    verify_token(tokens.tokens[5], TOKEN_KEYWORD, "let", 2, 1);
    verify_token(tokens.tokens[6], TOKEN_IDENTIFIER, "y", 2, 5);
    verify_token(tokens.tokens[7], TOKEN_OPERATOR, "=", 2, 7);
    verify_token(tokens.tokens[8], TOKEN_INTEGER, "10", 2, 9);
    verify_token(tokens.tokens[9], TOKEN_SEMICOLON, ";", 2, 11);

    free_token_array(&tokens);

    // Test comment on its own line
    const char *input2 = "let x = 42;\n// Comment on its own line\nlet y = 10;";
    tokens = tokenize(input2);

    verify_token(tokens.tokens[0], TOKEN_KEYWORD, "let", 1, 1);
    verify_token(tokens.tokens[1], TOKEN_IDENTIFIER, "x", 1, 5);
    verify_token(tokens.tokens[2], TOKEN_OPERATOR, "=", 1, 7);
    verify_token(tokens.tokens[3], TOKEN_INTEGER, "42", 1, 9);
    verify_token(tokens.tokens[4], TOKEN_SEMICOLON, ";", 1, 11);
    // Comment line should be skipped entirely
    verify_token(tokens.tokens[5], TOKEN_KEYWORD, "let", 3, 1);
    verify_token(tokens.tokens[6], TOKEN_IDENTIFIER, "y", 3, 5);
    verify_token(tokens.tokens[7], TOKEN_OPERATOR, "=", 3, 7);
    verify_token(tokens.tokens[8], TOKEN_INTEGER, "10", 3, 9);
    verify_token(tokens.tokens[9], TOKEN_SEMICOLON, ";", 3, 11);

    free_token_array(&tokens);

    // Test comment with f-string
    const char *input3 = "let msg = f\"Value is {x + 1}\"; // Comment after f-string\nlet y = 10;";
    tokens = tokenize(input3);

    verify_token(tokens.tokens[0], TOKEN_KEYWORD, "let", 1, 1);
    verify_token(tokens.tokens[1], TOKEN_IDENTIFIER, "msg", 1, 5);
    verify_token(tokens.tokens[2], TOKEN_OPERATOR, "=", 1, 9);
    verify_token(tokens.tokens[3], TOKEN_FSTRING, "Value is ", 1, 13);
    verify_token(tokens.tokens[4], TOKEN_IDENTIFIER, "x", 1, 21);
    verify_token(tokens.tokens[5], TOKEN_OPERATOR, "+", 1, 23);
    verify_token(tokens.tokens[6], TOKEN_INTEGER, "1", 1, 25);
    verify_token(tokens.tokens[7], TOKEN_SEMICOLON, ";", 1, 28);
    // Comment should be ignored, next token should be on next line
    verify_token(tokens.tokens[8], TOKEN_KEYWORD, "let", 2, 1);
    verify_token(tokens.tokens[9], TOKEN_IDENTIFIER, "y", 2, 5);
    verify_token(tokens.tokens[10], TOKEN_OPERATOR, "=", 2, 7);
    verify_token(tokens.tokens[11], TOKEN_INTEGER, "10", 2, 9);
    verify_token(tokens.tokens[12], TOKEN_SEMICOLON, ";", 2, 11);

    free_token_array(&tokens);

    printf("✓ Single line comments test passed\n");
}

int main()
{
    printf("Running token position tracking tests...\n");

    test_single_line();
    test_multi_line();
    test_string_positions();
    test_basic_fstring();
    test_numeric_fstring();
    test_identifier_fstring();
    test_multiline_fstring();
    test_single_line_comments();

    printf("All token position tracking tests passed!\n");
    return 0;
}