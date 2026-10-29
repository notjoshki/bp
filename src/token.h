#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_ID,
    TOKEN_INT,
    TOKEN_STAR,
    TOKEN_COLON,
    TOKEN_EQUAL,
    TOKEN_DOLLAR,
    TOKEN_GENERIC,
    TOKEN_NEWLINE,
    TOKEN_WHITESPACE,
    TOKEN_BSLASH,
    TOKEN_LPAREN,
    TOKEN_RPAREN
} Token_Type;

typedef struct {
    Token_Type type;
    char *value;
    size_t ln;
    size_t col;
} Token;

void delete_token(Token *token);
char *token_type_to_string(const Token_Type type);

#endif