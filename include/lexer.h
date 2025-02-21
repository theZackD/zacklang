#ifndef LEXER_H
#define LEXER_H

// Define different token types.
typedef enum
{
  TOKEN_IDENTIFIER, // Variable names.
  TOKEN_KEYWORD,    // Keywords.
  TOKEN_INTEGER,    // Numbers.
  TOKEN_FLOAT,      // Floating-point numbers.
  TOKEN_STRING,     // Regular string literals.
  TOKEN_FSTRING,    // f-string literals (with interpolation).
  TOKEN_OPERATOR,   // Operators.
  TOKEN_LPAREN,     // (
  TOKEN_RPAREN,     // )
  TOKEN_LBRACE,     // {
  TOKEN_RBRACE,     // }
  TOKEN_LBRACKET,   // [
  TOKEN_RBRACKET,   // ]
  TOKEN_SEMICOLON,  // ;
  TOKEN_COLON,      // :
  TOKEN_EOF         // End of file.
} TokenType;

// Token structure.
typedef struct
{
  TokenType type;
  char *value;
} Token;

// Token Array structure.
typedef struct
{
  Token *tokens;
  int count;
  int capacity;
} TokenArray;

// Function prototypes.
TokenArray create_token_array();
void add_token(TokenArray *array, TokenType type, const char *value);
void free_token_array(TokenArray *array);
TokenArray tokenize(const char *code);
void print_tokens(const TokenArray *array);

#endif // LEXER_H
