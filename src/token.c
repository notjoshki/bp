#include "token.h"
#include "error.h"
#include <stdlib.h>

void delete_token(Token *token) {
    if (token->type == TOKEN_ID || token->type == TOKEN_INT || token->type == TOKEN_GENERIC)
        free(token->value);
}

char *token_type_to_string(const Token_Type type) {
    switch (type) {
        case TOKEN_EOF: return "eof";
        case TOKEN_ID: return "id";
        case TOKEN_INT: return "int";
        case TOKEN_STAR: return "star";
        case TOKEN_COLON: return "colon";
        case TOKEN_EQUAL: return "equal";
        case TOKEN_DOLLAR: return "dollar";
        case TOKEN_GENERIC: return "generic";
        case TOKEN_NEWLINE: return "newline";
        case TOKEN_WHITESPACE: return "whitespace";
        case TOKEN_BSLASH: return "bslash";
        default:
            log_dev_error("Undefined token type");
            return "\0";
    }
}