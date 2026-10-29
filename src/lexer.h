#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stdio.h>

typedef struct {
    char *filename;
    char *source;
    size_t source_length;
    size_t source_index;
    char current_char;
    size_t current_ln;
    size_t current_col;
} Lexer;

Lexer create_lexer(char *filename);
void delete_lexer(Lexer *lexer);
Token get_next_token(Lexer *lexer);

#endif