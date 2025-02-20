#include "../../include/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Helper function to print token details for debugging
void print_token(Token token)
{
    printf("Token { type: %d, value: \"%s\" }\n", token.type, token.value);
}

// Test basic keywords and types
void test_keywords(void)
{
    const char *source = "let const fn if else while for i32 i64 f32 f64 bool string void";
    TokenArray tokens = tokenize(source);

    assert(tokens.count > 0);
    for (int i = 0; i < tokens.count - 1; i++)
    { // -1 to skip EOF
        assert(tokens.tokens[i].type == TOKEN_KEYWORD);
    }

    free_token_array(&tokens);
    printf("✓ Keywords test passed\n");
}

// Test array type declarations
void test_array_types(void)
{
    const char *source = "let arr: i32[] = 5\nlet matrix: f64[]";
    TokenArray tokens = tokenize(source);

    // Expected sequence: let, arr, :, i32, [, ], =, 5, let, matrix, :, f64, [, ]
    int expected_types[] = {
        TOKEN_KEYWORD,    // let
        TOKEN_IDENTIFIER, // arr
        TOKEN_COLON,      // :
        TOKEN_KEYWORD,    // i32
        TOKEN_LBRACKET,   // [
        TOKEN_RBRACKET,   // ]
        TOKEN_OPERATOR,   // =
        TOKEN_INTEGER,    // 5
        TOKEN_KEYWORD,    // let
        TOKEN_IDENTIFIER, // matrix
        TOKEN_COLON,      // :
        TOKEN_KEYWORD,    // f64
        TOKEN_LBRACKET,   // [
        TOKEN_RBRACKET    // ]
    };

    for (int i = 0; i < sizeof(expected_types) / sizeof(expected_types[0]); i++)
    {
        assert(tokens.tokens[i].type == expected_types[i]);
    }

    free_token_array(&tokens);
    printf("✓ Array types test passed\n");
}

// Test regular strings
void test_strings(void)
{
    const char *source = "\"Hello, World!\" \"String with \\\"escape\\\"\"";
    TokenArray tokens = tokenize(source);

    assert(tokens.count == 3); // 2 strings + EOF
    assert(tokens.tokens[0].type == TOKEN_STRING);
    assert(tokens.tokens[1].type == TOKEN_STRING);
    assert(strcmp(tokens.tokens[0].value, "Hello, World!") == 0);

    free_token_array(&tokens);
    printf("✓ Regular strings test passed\n");
}

// Test f-strings without interpolation
void test_basic_fstrings(void)
{
    const char *source = "f\"Hello, World!\" f\"No interpolation here\"";
    TokenArray tokens = tokenize(source);

    assert(tokens.tokens[0].type == TOKEN_FSTRING);
    assert(tokens.tokens[1].type == TOKEN_FSTRING);

    free_token_array(&tokens);
    printf("✓ Basic f-strings test passed\n");
}

// Test f-strings with interpolation
void test_fstring_interpolation(void)
{
    const char *source = "f\"Hello, {name}! Age: {age + 1}\"";
    TokenArray tokens = tokenize(source);

    // Print tokens for debugging
    printf("Tokens for interpolation test:\n");
    for (int i = 0; i < tokens.count; i++)
    {
        print_token(tokens.tokens[i]);
    }

    // Expected sequence: FSTRING("Hello, "), IDENTIFIER(name), FSTRING("! Age: "),
    // IDENTIFIER(age), OPERATOR(+), INTEGER(1), FSTRING("")
    assert(tokens.tokens[0].type == TOKEN_FSTRING);
    assert(tokens.tokens[1].type == TOKEN_IDENTIFIER);
    assert(tokens.tokens[2].type == TOKEN_FSTRING);
    assert(tokens.tokens[3].type == TOKEN_IDENTIFIER);
    assert(tokens.tokens[4].type == TOKEN_OPERATOR);
    assert(tokens.tokens[5].type == TOKEN_INTEGER);

    free_token_array(&tokens);
    printf("✓ F-string interpolation test passed\n");
}

// Test nested f-string interpolation
void test_nested_fstring_interpolation(void)
{
    const char *source = "f\"Nested: {f\"Inner: {x + 2}\"}\"";
    TokenArray tokens = tokenize(source);

    // Print tokens for debugging
    printf("Tokens for nested interpolation test:\n");
    for (int i = 0; i < tokens.count; i++)
    {
        print_token(tokens.tokens[i]);
    }

    free_token_array(&tokens);
    printf("✓ Nested f-string interpolation test passed\n");
}

// Test error cases
void test_error_handling(void)
{
    // Unterminated string
    const char *source1 = "\"Unterminated string";
    TokenArray tokens1 = tokenize(source1);

    // Unterminated f-string
    const char *source2 = "f\"Unterminated f-string";
    TokenArray tokens2 = tokenize(source2);

    // Invalid interpolation
    const char *source3 = "f\"Missing closing brace: {name\"";
    TokenArray tokens3 = tokenize(source3);

    free_token_array(&tokens1);
    free_token_array(&tokens2);
    free_token_array(&tokens3);
    printf("✓ Error handling test passed\n");
}

int main()
{
    printf("Running lexer tests...\n");

    test_keywords();
    test_array_types();
    test_strings();
    test_basic_fstrings();
    test_fstring_interpolation();
    test_nested_fstring_interpolation();
    test_error_handling();

    printf("All tests passed!\n");
    return 0;
}