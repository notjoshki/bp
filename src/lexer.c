#include "lexer.h"
#include "token.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ID_START_SIZE 8
#define INT_START_SIZE 4

Lexer create_lexer(char *filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        log_error("No such file '%s' exists", filename);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    const size_t size = ftell(file);
    rewind(file);

    char *source = malloc(size + 1);
    const size_t bytes_read = fread(source, sizeof(char), size, file);

    if (bytes_read != size || bytes_read <= 0) {
        log_error("Failed to read file '%s'", filename);
        free(source);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    source[bytes_read] = '\0';
    return (Lexer){ .filename = filename, .source = source, .source_length = bytes_read, .current_char = source[0],
        .source_index = 0, .current_ln = 1, .current_col = 1 };
}

void delete_lexer(Lexer *lexer) {
    free(lexer->source);
}

static void step(Lexer *lexer) {
    if (lexer->current_char == '\n') {
        lexer->current_ln++;
        lexer->current_col = 1;
    } else
        lexer->current_col++;

    lexer->current_char = lexer->source[++lexer->source_index];
}

static Token lex_id(Lexer *lexer) {
    const size_t ln = lexer->current_ln;
    const size_t col = lexer->current_col;
    char *value = malloc(ID_START_SIZE + 1);
    size_t len = 0;
    size_t cap = ID_START_SIZE;

    while (isalpha(lexer->current_char) || isdigit(lexer->current_char) || lexer->current_char == '_') {
        if (len + 1 > cap) {
            cap *= 2;
            value = realloc(value, cap + 1);
        }

        value[len++] = lexer->current_char;
        step(lexer);
    }

    value[len] = '\0';
    return (Token){ .type = TOKEN_ID, .value = value, .ln = ln, .col = col };
}

static Token lex_int(Lexer *lexer) {
    const size_t ln = lexer->current_ln;
    const size_t col = lexer->current_col;
    char *value = malloc(INT_START_SIZE + 1);
    size_t len = 0;
    size_t cap = INT_START_SIZE;

    while (isdigit(lexer->current_char)) {
        if (len + 1 > cap) {
            cap *= 2;
            value = realloc(value, cap + 1);
        }

        value[len++] = lexer->current_char;
        step(lexer);
    }

    value[len] = '\0';
    return (Token){ .type = TOKEN_INT, .value = value, .ln = ln, .col = col };
}

static Token step_and_create_token(Lexer *lexer, Token_Type type, char *value) {
    const size_t ln = lexer->current_ln;
    const size_t col = lexer->current_col;
    char *view = value;

    while (*view) {
        step(lexer);
        view++;
    }

    return (Token){ .type = type, .value = value, .ln = ln, .col = col };
}

static void skip_comment(Lexer *lexer) {
    while (lexer->current_char != '\0' && lexer->current_char != '\n')
        step(lexer);

    if (lexer->current_char == '\n')
        step(lexer);
}

Token get_next_token(Lexer *lexer) {
    if (lexer->current_char == ' ')
        return step_and_create_token(lexer, TOKEN_WHITESPACE, " ");

    if (isalpha(lexer->current_char) || lexer->current_char == '_')
        return lex_id(lexer);

    if (isdigit(lexer->current_char))
        return lex_int(lexer);

    switch (lexer->current_char) {
        case '\0': return (Token){ .type = TOKEN_EOF, .value = "\0", .ln = lexer->current_ln, .col = lexer->current_col };
        case '\n': return step_and_create_token(lexer, TOKEN_NEWLINE, "\n");
        case '#':
            skip_comment(lexer);
            return get_next_token(lexer);
        case '*': return step_and_create_token(lexer, TOKEN_STAR, "*");
        case ':': return step_and_create_token(lexer, TOKEN_COLON, ":");
        case '=': return step_and_create_token(lexer, TOKEN_EQUAL, "=");
        case '$': return step_and_create_token(lexer, TOKEN_DOLLAR, "$");
        case '\\': return step_and_create_token(lexer, TOKEN_BSLASH, "\\");
        case '(': return step_and_create_token(lexer, TOKEN_LPAREN, "(");
        case ')': return step_and_create_token(lexer, TOKEN_RPAREN, ")");
        default: break;
    }

    char *value = malloc(2);
    value[0] = lexer->current_char;
    value[1] = '\0';
    return step_and_create_token(lexer, TOKEN_GENERIC, value);
}