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

// Error-reporting helper (prints to stderr)
static void report_error(int line, int col, const char *message)
{
  fprintf(stderr, "Lexer Error (line %d, col %d): %s\n", line, col, message);
}

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
    fprintf(stderr, "Memory allocation failed for TokenArray\n");
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
      fprintf(stderr, "Memory allocation failed for expanding TokenArray\n");
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
  for (int i = 0; i < array->count; i++)
  {
    free(array->tokens[i].value);
  }
  free(array->tokens);
}

// Forward declaration for recursive tokenization in f-string interpolation.
TokenArray tokenize(const char *code);

// Tokenize the input string with robust line/column tracking.
TokenArray tokenize(const char *code)
{
  TokenArray array = create_token_array();
  size_t i = 0;
  int line = 1;
  int col = 1;

  while (code[i] != '\0')
  {
    // Skip whitespace and update position.
    while (isspace(code[i]))
    {
      if (code[i] == '\n')
      {
        line++;
        col = 1;
      }
      else
      {
        col++;
      }
      i++;
    }
    if (code[i] == '\0')
      break;

    // f-string literal: check for f" prefix.
    if (code[i] == 'f' && code[i + 1] == '"')
    {
      i += 2;
      col += 2; // Skip f"
      size_t start = i;
      while (code[i] != '"' && code[i] != '\0')
      {
        if (code[i] == '\n')
        {
          line++;
          col = 1;
        }
        else
        {
          col++;
        }

        if (code[i] == '{')
        {
          // Add the string part before the interpolation if it exists.
          if (i > start)
          {
            char *str_part = strndup(&code[start], i - start);
            add_token(&array, TOKEN_FSTRING, str_part);
            free(str_part);
          }

          i++;
          col++; // Skip '{'
          size_t expr_start = i;
          int brace_count = 1;

          // Find matching '}'.
          while (code[i] != '\0' && brace_count > 0)
          {
            if (code[i] == '\n')
            {
              line++;
              col = 1;
            }
            else
            {
              col++;
            }
            if (code[i] == '{')
              brace_count++;
            if (code[i] == '}')
              brace_count--;
            i++;
          }

          if (brace_count > 0)
          {
            report_error(line, col, "Unterminated interpolation in f-string");
            break;
          }

          // Back up one position so that we extract the expression correctly.
          i--;
          col--;
          size_t expr_len = i - expr_start;
          char *expr = strndup(&code[expr_start], expr_len);

          // Tokenize the expression.
          TokenArray expr_tokens = tokenize(expr);
          // Add all tokens except the EOF token.
          for (int j = 0; j < expr_tokens.count - 1; j++)
          {
            add_token(&array, expr_tokens.tokens[j].type, expr_tokens.tokens[j].value);
          }
          free_token_array(&expr_tokens);
          free(expr);

          i++;
          col++; // Move past the closing '}'
          start = i;
          continue;
        }

        // Handle escape sequences.
        if (code[i] == '\\' && code[i + 1] != '\0')
        {
          i++;
          col++;
        }
        i++;
      }

      // Add any remaining string part.
      if (i > start)
      {
        char *str_part = strndup(&code[start], i - start);
        add_token(&array, TOKEN_FSTRING, str_part);
        free(str_part);
      }

      if (code[i] == '"')
      {
        i++;
        col++; // Skip closing quote.
      }
      else
      {
        report_error(line, col, "Unterminated f-string");
      }
      continue;
    }

    // Regular string literal.
    if (code[i] == '"')
    {
      i++;
      col++; // Skip the opening quote.
      size_t start = i;
      while (code[i] != '"' && code[i] != '\0')
      {
        if (code[i] == '\\' && code[i + 1] != '\0')
        {
          i++;
          col++;
        }
        if (code[i] == '\n')
        {
          line++;
          col = 1;
        }
        else
        {
          col++;
        }
        i++;
      }
      if (code[i] == '\0')
      {
        report_error(line, col, "Unterminated string literal");
        break;
      }
      char *string_val = strndup(&code[start], i - start);
      add_token(&array, TOKEN_STRING, string_val);
      free(string_val);
      i++;
      col++; // Skip the closing quote.
      continue;
    }

    // Identifiers & Keywords (start with a letter or underscore).
    if (isalpha(code[i]) || code[i] == '_')
    {
      size_t start = i;
      while (isalnum(code[i]) || code[i] == '_')
      {
        i++;
        col++;
      }
      char *value = strndup(&code[start], i - start);
      TokenType type = is_keyword(value) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
      add_token(&array, type, value);
      free(value);
      continue;
    }

    // Numbers (Integers, Floats, and scientific notation).
    if (isdigit(code[i]) || (code[i] == '.' && isdigit(code[i + 1])))
    {
      size_t start = i;
      int has_dot = 0;
      int has_e = 0;
      while (isdigit(code[i]) ||
             (code[i] == '.' && !has_dot) ||
             ((code[i] == 'e' || code[i] == 'E') && !has_e) ||
             ((code[i] == '+' || code[i] == '-') && i > start && (code[i - 1] == 'e' || code[i - 1] == 'E')))
      {
        if (code[i] == '.')
        {
          has_dot = 1;
        }
        if (code[i] == 'e' || code[i] == 'E')
        {
          has_e = 1;
          has_dot = 1; // Force float type for scientific notation.
        }
        i++;
        col++;
      }
      char *num = strndup(&code[start], i - start);
      add_token(&array, has_dot ? TOKEN_FLOAT : TOKEN_INTEGER, num);
      free(num);
      continue;
    }

    // Multi-Character Operators.
    int matched = 0;
    for (size_t op = 0; op < NUM_OPERATORS; op++)
    {
      size_t opLen = strlen(MULTI_CHAR_OPERATORS[op]);
      if (strncmp(&code[i], MULTI_CHAR_OPERATORS[op], opLen) == 0)
      {
        add_token(&array, TOKEN_OPERATOR, MULTI_CHAR_OPERATORS[op]);
        // Update column (and line if any newline appears in the operator).
        for (size_t k = 0; k < opLen; k++)
        {
          if (code[i + k] == '\n')
          {
            line++;
            col = 1;
          }
          else
          {
            col++;
          }
        }
        i += opLen;
        matched = 1;
        break;
      }
    }
    if (matched)
      continue;

    // Single-Character Operators and Symbols.
    char current = code[i];
    switch (current)
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
      if (is_single_char_operator(current))
      {
        char op_single[2] = {current, '\0'};
        add_token(&array, TOKEN_OPERATOR, op_single);
      }
      else
      {
        char err_msg[64];
        snprintf(err_msg, sizeof(err_msg), "Unexpected character '%c'", current);
        report_error(line, col, err_msg);
      }
      break;
    }
    i++;
    col++;
  }

  add_token(&array, TOKEN_EOF, "EOF"); // Append end-of-file token.
  return array;
}

// Helper: Convert TokenType to a human-readable string.
const char *token_type_to_string(TokenType type)
{
  switch (type)
  {
  case TOKEN_IDENTIFIER:
    return "Identifier";
  case TOKEN_KEYWORD:
    return "Keyword";
  case TOKEN_INTEGER:
    return "Integer";
  case TOKEN_FLOAT:
    return "Float";
  case TOKEN_STRING:
    return "String";
  case TOKEN_FSTRING:
    return "FString";
  case TOKEN_OPERATOR:
    return "Operator";
  case TOKEN_LPAREN:
    return "LeftParen";
  case TOKEN_RPAREN:
    return "RightParen";
  case TOKEN_LBRACE:
    return "LeftBrace";
  case TOKEN_RBRACE:
    return "RightBrace";
  case TOKEN_LBRACKET:
    return "LeftBracket";
  case TOKEN_RBRACKET:
    return "RightBracket";
  case TOKEN_SEMICOLON:
    return "Semicolon";
  case TOKEN_COLON:
    return "Colon";
  case TOKEN_EOF:
    return "EOF";
  default:
    return "Unknown";
  }
}

// Print tokens (useful for debugging)
void print_tokens(const TokenArray *array)
{
  for (int i = 0; i < array->count; i++)
  {
    printf("Token[%d]: Type = %s, Value = %s\n", i,
           token_type_to_string(array->tokens[i].type),
           array->tokens[i].value);
  }
}
