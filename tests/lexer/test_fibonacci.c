#include "../../include/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void print_token(Token token)
{
    const char *type_names[] = {
        "IDENTIFIER", "KEYWORD", "INTEGER", "FLOAT", "STRING",
        "FSTRING", "OPERATOR", "LPAREN", "RPAREN", "LBRACE",
        "RBRACE", "LBRACKET", "RBRACKET", "SEMICOLON", "COLON",
        "EOF"};
    printf("Token { type: %s, value: \"%s\" }\n", type_names[token.type], token.value);
}

void test_fibonacci_implementation(void)
{
    // A sample fibonacci implementation in Zacklang with comptime evaluation
    const char *source =
        "comptime fn fibonacci(n: i32): i32 {\n"
        "    if (n <= 1) {\n"
        "        return n\n"
        "    }\n"
        "    return fibonacci(n - 1) + fibonacci(n - 2)\n"
        "}\n"
        "\n"
        "fn main(): void {\n"
        "    let const fib10: i32 = fibonacci(10)\n"
        "    print(f\"The 10th fibonacci number is: {fib10}\")\n"
        "    \n"
        "    // Calculate first 5 fibonacci numbers\n"
        "    let results: i32[] = [fibonacci(0), fibonacci(1),\n"
        "                         fibonacci(2), fibonacci(3),\n"
        "                         fibonacci(4)]\n"
        "    \n"
        "    for (i in {0:5}) {\n"
        "        print(f\"Fibonacci({i}) = {results[i]}\")\n"
        "    }\n"
        "}\n";

    TokenArray tokens = tokenize(source);

    // Print all tokens for inspection
    printf("\nTokenizing fibonacci implementation:\n");
    printf("=====================================\n");
    for (int i = 0; i < tokens.count; i++)
    {
        print_token(tokens.tokens[i]);
    }

    // Verify key elements
    int i = 0;

    // Check comptime function declaration
    assert(strcmp(tokens.tokens[i++].value, "comptime") == 0);
    assert(strcmp(tokens.tokens[i++].value, "fn") == 0);
    assert(strcmp(tokens.tokens[i++].value, "fibonacci") == 0);
    assert(tokens.tokens[i++].type == TOKEN_LPAREN);
    assert(strcmp(tokens.tokens[i++].value, "n") == 0);
    assert(tokens.tokens[i++].type == TOKEN_COLON);
    assert(strcmp(tokens.tokens[i++].value, "i32") == 0);
    assert(tokens.tokens[i++].type == TOKEN_RPAREN);
    assert(tokens.tokens[i++].type == TOKEN_COLON);
    assert(strcmp(tokens.tokens[i++].value, "i32") == 0);
    assert(tokens.tokens[i++].type == TOKEN_LBRACE);

    // Find and verify f-string token sequence
    bool found_fstring = false;
    for (int j = 0; j < tokens.count - 3; j++)
    {
        if (tokens.tokens[j].type == TOKEN_FSTRING &&
            tokens.tokens[j + 1].type == TOKEN_IDENTIFIER &&
            strcmp(tokens.tokens[j + 1].value, "fib10") == 0)
        {
            found_fstring = true;
            break;
        }
    }
    assert(found_fstring && "F-string interpolation not found");

    // Verify array type declaration
    bool found_array_type = false;
    for (int j = 0; j < tokens.count - 2; j++)
    {
        if (tokens.tokens[j].type == TOKEN_KEYWORD &&
            strcmp(tokens.tokens[j].value, "i32") == 0 &&
            tokens.tokens[j + 1].type == TOKEN_LBRACKET)
        {
            found_array_type = true;
            break;
        }
    }
    assert(found_array_type && "Array type declaration not found");

    free_token_array(&tokens);
    printf("\n✓ Fibonacci implementation tokenization test passed\n");
}

int main()
{
    printf("Testing fibonacci implementation tokenization...\n");
    test_fibonacci_implementation();
    return 0;
}