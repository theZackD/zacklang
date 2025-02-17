#include "include/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_token(Token token)
{
  printf("Token { type: %d, value: '%s' }\n", token.type, token.value);
}

void test_variable_declaration()
{
  printf("\nTesting variable declaration...\n");
  const char *source = "let x: i32 = 42;";
  TokenArray tokens = tokenize(source);

  assert(tokens.count == 8);
  assert(strcmp(tokens.tokens[0].value, "let") == 0);
  assert(strcmp(tokens.tokens[1].value, "x") == 0);
  assert(strcmp(tokens.tokens[2].value, ":") == 0);
  assert(strcmp(tokens.tokens[3].value, "i32") == 0);
  assert(strcmp(tokens.tokens[4].value, "=") == 0);
  assert(strcmp(tokens.tokens[5].value, "42") == 0);
  assert(strcmp(tokens.tokens[6].value, ";") == 0);

  printf("✓ Variable declaration test passed\n");
  free_token_array(&tokens);
}

void test_arithmetic_expression()
{
  printf("\nTesting arithmetic expression...\n");
  const char *source = "2 + 3 * -4 / 5;";
  TokenArray tokens = tokenize(source);

  for (int i = 0; i < tokens.count; i++)
  {
    print_token(tokens.tokens[i]);
  }

  assert(tokens.count == 10);                       // 2 + 3 * - 4 / 5 ; EOF (10 tokens total)
  assert(tokens.tokens[0].type == TOKEN_INTEGER);   // 2
  assert(tokens.tokens[1].type == TOKEN_OPERATOR);  // +
  assert(tokens.tokens[2].type == TOKEN_INTEGER);   // 3
  assert(tokens.tokens[3].type == TOKEN_OPERATOR);  // *
  assert(tokens.tokens[4].type == TOKEN_OPERATOR);  // -
  assert(tokens.tokens[5].type == TOKEN_INTEGER);   // 4
  assert(tokens.tokens[6].type == TOKEN_OPERATOR);  // /
  assert(tokens.tokens[7].type == TOKEN_INTEGER);   // 5
  assert(tokens.tokens[8].type == TOKEN_SEMICOLON); // ;
  assert(tokens.tokens[9].type == TOKEN_EOF);       // EOF

  printf("✓ Arithmetic expression test passed\n");
  free_token_array(&tokens);
}

void test_string_literals()
{
  printf("\nTesting string literals...\n");
  const char *source = "\"Hello, World!\" f\"Interpolated {x}\"";
  TokenArray tokens = tokenize(source);

  for (int i = 0; i < tokens.count; i++)
  {
    print_token(tokens.tokens[i]);
  }

  assert(tokens.tokens[0].type == TOKEN_STRING);
  assert(tokens.tokens[1].type == TOKEN_FSTRING);

  printf("✓ String literals test passed\n");
  free_token_array(&tokens);
}

void test_keywords_and_identifiers()
{
  printf("\nTesting keywords and identifiers...\n");
  const char *source = "if true { print x } else { return false }";
  TokenArray tokens = tokenize(source);

  for (int i = 0; i < tokens.count; i++)
  {
    print_token(tokens.tokens[i]);
  }

  assert(tokens.tokens[0].type == TOKEN_KEYWORD);    // if
  assert(tokens.tokens[1].type == TOKEN_KEYWORD);    // true
  assert(tokens.tokens[3].type == TOKEN_KEYWORD);    // print
  assert(tokens.tokens[4].type == TOKEN_IDENTIFIER); // x

  printf("✓ Keywords and identifiers test passed\n");
  free_token_array(&tokens);
}

void test_number_edge_cases()
{
  printf("\nTesting number edge cases...\n");
  const char *source = "3.14 -2.5 0.0 42. .5 001 1e5;";
  TokenArray tokens = tokenize(source);

  for (int i = 0; i < tokens.count; i++)
  {
    print_token(tokens.tokens[i]);
  }

  assert(tokens.tokens[0].type == TOKEN_FLOAT);     // 3.14
  assert(tokens.tokens[1].type == TOKEN_OPERATOR);  // -
  assert(tokens.tokens[2].type == TOKEN_FLOAT);     // 2.5
  assert(tokens.tokens[3].type == TOKEN_FLOAT);     // 0.0
  assert(tokens.tokens[4].type == TOKEN_FLOAT);     // 42.
  assert(tokens.tokens[5].type == TOKEN_FLOAT);     // .5
  assert(tokens.tokens[6].type == TOKEN_INTEGER);   // 001
  assert(tokens.tokens[7].type == TOKEN_FLOAT);     // 1e5
  assert(tokens.tokens[8].type == TOKEN_SEMICOLON); // ;
  assert(tokens.tokens[9].type == TOKEN_EOF);       // EOF

  printf("✓ Number edge cases test passed\n");
  free_token_array(&tokens);
}

void test_string_edge_cases()
{
  printf("\nTesting string edge cases...\n");
  const char *source = "\"\" \"Hello\\nWorld\" \"Escaped \\\"Quote\\\"\" f\"Value: {x + y}\"";
  TokenArray tokens = tokenize(source);

  for (int i = 0; i < tokens.count; i++)
  {
    print_token(tokens.tokens[i]);
  }

  assert(tokens.tokens[0].type == TOKEN_STRING);  // Empty string
  assert(tokens.tokens[1].type == TOKEN_STRING);  // String with newline
  assert(tokens.tokens[2].type == TOKEN_STRING);  // String with escaped quotes
  assert(tokens.tokens[3].type == TOKEN_FSTRING); // F-string with expression

  printf("✓ String edge cases test passed\n");
  free_token_array(&tokens);
}

void test_operators()
{
  printf("\nTesting operators...\n");
  const char *source = "== != <= >= and or not xor + - * / % ** 2**3;";
  TokenArray tokens = tokenize(source);

  for (int i = 0; i < tokens.count; i++)
  {
    print_token(tokens.tokens[i]);
  }

  assert(tokens.tokens[0].type == TOKEN_OPERATOR);    // ==
  assert(tokens.tokens[1].type == TOKEN_OPERATOR);    // !=
  assert(tokens.tokens[2].type == TOKEN_OPERATOR);    // <=
  assert(tokens.tokens[3].type == TOKEN_OPERATOR);    // >=
  assert(tokens.tokens[4].type == TOKEN_KEYWORD);     // and
  assert(tokens.tokens[5].type == TOKEN_KEYWORD);     // or
  assert(tokens.tokens[6].type == TOKEN_KEYWORD);     // not
  assert(tokens.tokens[7].type == TOKEN_KEYWORD);     // xor
  assert(tokens.tokens[8].type == TOKEN_OPERATOR);    // +
  assert(tokens.tokens[9].type == TOKEN_OPERATOR);    // -
  assert(tokens.tokens[10].type == TOKEN_OPERATOR);   // *
  assert(tokens.tokens[11].type == TOKEN_OPERATOR);   // /
  assert(tokens.tokens[12].type == TOKEN_OPERATOR);   // %
  assert(tokens.tokens[13].type == TOKEN_OPERATOR);   // **
  assert(strcmp(tokens.tokens[13].value, "**") == 0); // Verify it's the power operator
  // Test power operator in use
  assert(tokens.tokens[14].type == TOKEN_INTEGER);  // 2
  assert(tokens.tokens[15].type == TOKEN_OPERATOR); // **
  assert(tokens.tokens[16].type == TOKEN_INTEGER);  // 3

  printf("✓ Operators test passed\n");
  free_token_array(&tokens);
}

int main(void)
{
  printf("Running lexer tests...\n");

  test_variable_declaration();
  test_arithmetic_expression();
  test_string_literals();
  test_keywords_and_identifiers();
  test_number_edge_cases();
  test_string_edge_cases();
  test_operators();

  printf("\nAll tests passed! ✓\n");
  return 0;
}
