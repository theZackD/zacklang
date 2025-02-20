#include "../include/lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Defining ZackLang keywords:
const char *KEYWORDS[] = {"let", "const", "print", "prompt", "if",
                          "else", "elif", "case", "switch", "finally",
                          "true", "false", "fn", "return", "break",
                          "continue", "while", "for", "and", "or",
                          "not", "xor", "in", "struct", // Primitive types:
                          "i32", "i64", "f32", "f64", "bool",
                          "char", "string", "void", "comptime"};
#define NUM_KEYWORDS (sizeof(KEYWORDS) / sizeof(KEYWORDS[0]))

// Defining multi-character operators:
const char *MULTI_CHAR_OPERATORS[] = {"==", "!=", "<=", ">=", "//", "**"};
#define NUM_OPERATORS (sizeof(MULTI_CHAR_OPERATORS) / sizeof(MULTI_CHAR_OPERATORS[0]))

// Defining single-character operators & symbols:
const char SINGLE_CHAR_OPERATORS[] = {'+', '-', '*', '/', '%', '=',
                                      '<', '>', '&', '|', ';', ':',
                                      '(', ')', '{', '}', '[', ']',
                                      ','};
#define NUM_SINGLE_OPERATORS (sizeof(SINGLE_CHAR_OPERATORS) / sizeof(SINGLE_CHAR_OPERATORS[0]))

// Check if a string is a keyword (including primitive types)
int is_keyword(const char *str)
{
  for (size_t i = 0; i < NUM_KEYWORDS; i++)
  {
    if (strcmp(str, KEYWORDS[i]) == 0)
    {
      return 1; // It's a keyword
    }
  }
  return 0; // Not a keyword
}

// Check if a string is a multi-character operator
int is_multi_char_operator(const char *str)
{
  for (size_t i = 0; i < NUM_OPERATORS; i++)
  {
    if (strcmp(str, MULTI_CHAR_OPERATORS[i]) == 0)
    {
      return 1; // It's a multi-character operator
    }
  }
  return 0; // Not found
}

// Check if a character is a single-character operator or symbol
int is_single_char_operator(char c)
{
  for (size_t i = 0; i < NUM_SINGLE_OPERATORS; i++)
  {
    if (SINGLE_CHAR_OPERATORS[i] == c)
    {
      return 1; // It's a single-character operator/symbol
    }
  }
  return 0; // Not found
}

// Initialize tokenArray
#define INITIAL_CAPACITY 16 // Start with 16 tokens, expand dynamically
TokenArray create_token_array()
{
  TokenArray array;
  array.tokens = malloc(INITIAL_CAPACITY * sizeof(Token));
  if (!array.tokens)
  {
    printf("Memory allocation failed for TokenArray\n");
    exit(1);
  }
  array.count = 0;
  array.capacity = INITIAL_CAPACITY;
  return array;
}

// Add a token to the tokenArray
void add_token(TokenArray *array, TokenType type, const char *value)
{
  if (array->count >= array->capacity)
  {
    array->capacity *= 2;
    array->tokens = realloc(array->tokens, array->capacity * sizeof(Token));
    if (!array->tokens)
    {
      printf("Memory allocation failed for expanding TokenArray\n");
      exit(1);
    }
  }
  array->tokens[array->count].type = type;
  array->tokens[array->count].value = strdup(value);
  array->count++;
}

// Free the memory allocated for the tokenArray
void free_token_array(TokenArray *array)
{
  for (size_t i = 0; i < array->count; i++)
  {
    free(array->tokens[i].value);
  }
  free(array->tokens);
}

// Tokenize the input string
TokenArray tokenize(const char *code)
{
  TokenArray array = create_token_array();
  size_t i = 0;

  while (code[i] != '\0')
  {
    // Skip whitespace
    while (isspace(code[i]))
    {
      i++;
    }
    if (code[i] == '\0')
      break;

    // f-string literal: check for f" prefix.
    if (code[i] == 'f' && code[i + 1] == '"')
    {
      i += 2; // Skip f"
      size_t start = i;

      while (code[i] != '"' && code[i] != '\0')
      {
        if (code[i] == '{')
        {
          // Add the string part before the interpolation if it exists
          if (i > start)
          {
            char *str_part = strndup(&code[start], i - start);
            add_token(&array, TOKEN_FSTRING, str_part);
            free(str_part);
          }

          i++; // Skip {
          size_t expr_start = i;
          int brace_count = 1;

          // Find matching }
          while (code[i] != '\0' && brace_count > 0)
          {
            if (code[i] == '{')
              brace_count++;
            if (code[i] == '}')
              brace_count--;
            i++;
          }

          if (brace_count > 0)
          {
            printf("Lexer Error: Unterminated interpolation in f-string\n");
            break;
          }

          i--; // Back up to the closing }
          // Extract the expression between { and }
          size_t expr_len = i - expr_start;
          char *expr = strndup(&code[expr_start], expr_len);

          // Tokenize the expression
          TokenArray expr_tokens = tokenize(expr);
          // Add all tokens except EOF from the expression
          for (int j = 0; j < expr_tokens.count - 1; j++)
          {
            add_token(&array, expr_tokens.tokens[j].type, expr_tokens.tokens[j].value);
          }
          free_token_array(&expr_tokens);
          free(expr);

          i++; // Move past the closing }
          start = i;
          continue;
        }

        // Handle escape sequences
        if (code[i] == '\\' && code[i + 1] != '\0')
        {
          i++;
        }
        i++;
      }

      // Add remaining string part if it exists
      if (i > start)
      {
        char *str_part = strndup(&code[start], i - start);
        add_token(&array, TOKEN_FSTRING, str_part);
        free(str_part);
      }

      if (code[i] == '"')
      {
        i++;
      }
      else
      {
        printf("Lexer Error: Unterminated f-string\n");
      }
      continue;
    }

    // Regular string literal
    if (code[i] == '"')
    {
      i++; // Skip the opening quote
      size_t start = i;
      while (code[i] != '"' && code[i] != '\0')
      {
        // Handle escape sequences
        if (code[i] == '\\' && code[i + 1] != '\0')
        {
          i++;
        }
        i++;
      }
      if (code[i] == '\0')
      {
        printf("Lexer Error: Unterminated string literal\n");
        break;
      }
      char *string_val = strndup(&code[start], i - start);
      add_token(&array, TOKEN_STRING, string_val);
      free(string_val);
      i++; // Skip the closing quote
      continue;
    }

    // Identifiers & Keywords (start with a letter or underscore)
    if (isalpha(code[i]) || code[i] == '_')
    {
      size_t start = i;
      while (isalnum(code[i]) || code[i] == '_')
      {
        i++;
      }
      char *value = strndup(&code[start], i - start);
      TokenType type = is_keyword(value) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
      add_token(&array, type, value);
      free(value);
      continue;
    }

    // Numbers (Integers, Floats, and Negatives)
    if (isdigit(code[i]) || (code[i] == '.' && isdigit(code[i + 1])))
    {
      size_t start = i;
      int has_dot = 0;
      int has_e = 0;
      while (isdigit(code[i]) ||
             (code[i] == '.' && !has_dot) ||
             ((code[i] == 'e' || code[i] == 'E') && !has_e) ||
             ((code[i] == '+' || code[i] == '-') && (code[i - 1] == 'e' || code[i - 1] == 'E')))
      {
        if (code[i] == '.')
        {
          has_dot = 1;
        }
        if (code[i] == 'e' || code[i] == 'E')
        {
          has_e = 1;
          has_dot = 1; // Force float type for scientific notation
        }
        i++;
      }
      char *num = strndup(&code[start], i - start);
      add_token(&array, has_dot ? TOKEN_FLOAT : TOKEN_INTEGER, num);
      free(num);
      continue;
    }

    // Multi-Character Operators (variable length)
    int matched = 0;
    for (size_t op = 0; op < NUM_OPERATORS; op++)
    {
      size_t opLen = strlen(MULTI_CHAR_OPERATORS[op]);
      if (strncmp(&code[i], MULTI_CHAR_OPERATORS[op], opLen) == 0)
      {
        add_token(&array, TOKEN_OPERATOR, MULTI_CHAR_OPERATORS[op]);
        i += opLen;
        matched = 1;
        break;
      }
    }
    if (matched)
      continue;

    // Single-Character Operators and Symbols, including brackets and parentheses
    switch (code[i])
    {
    case '(':
      add_token(&array, TOKEN_LPAREN, "(");
      break;
    case ')':
      add_token(&array, TOKEN_RPAREN, ")");
      break;
    case '{':
      add_token(&array, TOKEN_LBRACE, "{");
      break;
    case '}':
      add_token(&array, TOKEN_RBRACE, "}");
      break;
    case '[':
      add_token(&array, TOKEN_LBRACKET, "[");
      break;
    case ']':
      add_token(&array, TOKEN_RBRACKET, "]");
      break;
    case ';':
      add_token(&array, TOKEN_SEMICOLON, ";");
      break;
    case ':':
      add_token(&array, TOKEN_COLON, ":");
      break;
    default:
      if (is_single_char_operator(code[i]))
      {
        char op_single[2] = {code[i], '\0'};
        add_token(&array, TOKEN_OPERATOR, op_single);
      }
      else
      {
        printf("Lexer Error: Unexpected character '%c'\n", code[i]);
      }
      break;
    }
    i++; // Move past the current character
  }

  add_token(&array, TOKEN_EOF, "EOF"); // Append end-of-file token
  return array;
}
