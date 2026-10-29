#include "parser.h"
#include "blueprint.h"
#include "token.h"
#include "lexer.h"
#include "error.h"
#include "macro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>

#define TOKENS_START_SIZE 16
#define CONSTANT_VALUE_START_SIZE 16
#define COMMAND_START_SIZE 32

typedef struct {
    char *filename;
    Blueprint *blueprint;
    Token *tokens;
    size_t token_count;
    size_t token_index;
    Token *token;
} Parser;

static Parser create_parser(char *filename, Blueprint *blueprint) {
    Lexer lex = create_lexer(filename);
    Token tok;

    Token *tokens = malloc(TOKENS_START_SIZE * sizeof(Token));
    size_t count = 0;
    size_t cap = TOKENS_START_SIZE;

    while ((tok = get_next_token(&lex)).type != TOKEN_EOF) {
        if (count + 2 > cap) {
            cap *= 2;
            tokens = realloc(tokens, cap * sizeof(Token));
        }

        tokens[count++] = tok;
    }

    tokens[count++] = tok; // EOF.
    delete_lexer(&lex);

    return (Parser){ .filename = filename, .blueprint = blueprint, .tokens = tokens, 
        .token_count = count, .token_index = 0, .token = &tokens[0] };
}

static void delete_parser(Parser *parser) {
    for (size_t i = 0; i < parser->token_count; i++)
        delete_token(&parser->tokens[i]);

    free(parser->tokens);
}

static void step(Parser *parser, const bool skip_whitespace) {
    if (parser->token->type != TOKEN_EOF)
        parser->token = &parser->tokens[++parser->token_index];

    if (!skip_whitespace)
        return;

    while (parser->token->type == TOKEN_WHITESPACE)
        step(parser, false);
}

static void eat(Parser *parser, const Token_Type type) {
    if (type != parser->token->type)
        log_error("Expected token '%s' but found '%s' (%s)", token_type_to_string(type), 
            parser->token->value, token_type_to_string(parser->token->type));

    step(parser, true);
}

static Token *peek(Parser *parser, const int offset) {
    if (parser->token_index + offset >= parser->token_count)
        return &parser->tokens[parser->token_count - 1]; // EOF.

    Token *tok;

    if ((int)parser->token_index + offset < 1)
        tok = &parser->tokens[0];
    else
        tok = &parser->tokens[parser->token_index + offset];

    if (tok->type == TOKEN_WHITESPACE)
        return peek(parser, offset > 0 ? offset + 1 : offset - 1);

    return tok;
}

static void step_to_next_line(Parser *parser) {
    while (parser->token->type != TOKEN_EOF && parser->token->type != TOKEN_NEWLINE)
        step(parser, true);

    if (parser->token->type == TOKEN_NEWLINE)
        step(parser, true);
}

static char *parse_value(Parser *parser);
static char *parse_values_until_token(Parser *parser, const Token_Type type);

static void parse_constant(Parser *parser) {
    if (find_constant(parser->token->value, parser->blueprint) != NULL) {
        log_error("Redefinition of constant '%s'", parser->token->value);
        step_to_next_line(parser);
        return;
    }

    char *name = malloc(strlen(parser->token->value) + 1);
    strcpy(name, parser->token->value);
    eat(parser, TOKEN_ID);
    eat(parser, TOKEN_EQUAL);
    push_constant(name, parse_values_until_token(parser, TOKEN_NEWLINE), parser->blueprint);
}

static char *parse_macro_find(Parser *parser) {
    eat(parser, TOKEN_ID);
    eat(parser, TOKEN_LPAREN);
    char *path = parse_values_until_token(parser, TOKEN_RPAREN);
    eat(parser, TOKEN_RPAREN);

    char *command = macro_find(path);
    free(path);
    return command;
}

static char *parse_macro(Parser *parser) {
    if (strcmp(parser->token->value, "find") == 0)
        return parse_macro_find(parser);

    log_error("Undefined macro '%s'", parser->token->value);
    return NULL;
}

static char *parse_value(Parser *parser) {
    char *src;

    if (parser->token->type == TOKEN_BSLASH && peek(parser, 1)->type == TOKEN_DOLLAR) {
        src = malloc(2);
        strcpy(src, "$");
        step(parser, false);
    } else if (parser->token->type == TOKEN_DOLLAR) {
        step(parser, true);
        const Constant *constant = find_constant(parser->token->value, parser->blueprint);

        if (constant == NULL) {
            log_error("Undefined constant '%s'", parser->token->value);
            step_to_next_line(parser);
            return calloc(1, sizeof(char));
        }

        src = malloc(strlen(constant->value) + 1);
        strcpy(src, constant->value);
        step(parser, false);
    } else if (parser->token->type != TOKEN_BSLASH && peek(parser, 1)->type == TOKEN_LPAREN) {
        src = parse_macro(parser);

        if (src == NULL) {
            step_to_next_line(parser);
            src = calloc(1, sizeof(char));
        }
    } else {
        src = malloc(strlen(parser->token->value) + 1);
        strcpy(src, parser->token->value);
        step(parser, false);
    }

    return src;
}

static char *parse_values_until_token(Parser *parser, const Token_Type type) {
    char *value = malloc(COMMAND_START_SIZE + 1);
    value[0] = '\0';
    size_t len = 0;
    size_t cap = COMMAND_START_SIZE;

    while (parser->token->type != TOKEN_EOF && parser->token->type != type) {
        char *src = parse_value(parser);
        const size_t src_len = strlen(src);

        if (len + src_len + 1 > cap) {
            cap *= 2;
            value = realloc(value, cap + 2);
        }

        strcat(value, src);
        free(src);
        len += src_len;

        if (parser->token->type == TOKEN_WHITESPACE && len > 0 && value[len - 1] != ' ') {
            strcat(value, " ");
            len++;
            step(parser, false);
        }
    }

    return value;
}

static void parse_rule(Parser *parser) {
    char *endptr;
    const long value = strtol(parser->token->value, &endptr, 10);

    if (endptr == parser->token->value || *endptr != '\0' || errno == ERANGE) {
        log_error("Failed to parse rule order '%s'", parser->token->value);

        if (*endptr == '\0')
            fprintf(stderr, ": %s\n", strerror(errno));

        step_to_next_line(parser);
        return;
    }

    if (value < 0) {
        log_error("Rule %ld is negative; lowest rule order is 0", value);
        step_to_next_line(parser);
        return;
    }

    if (find_rule((size_t)value, parser->blueprint) != NULL) {
        log_error("Redefinition of rule %ld", value);
        step_to_next_line(parser);
        return;
    }

    step(parser, true);
    eat(parser, TOKEN_COLON);
    push_rule((size_t)value, parse_values_until_token(parser, TOKEN_NEWLINE), parser->blueprint);
}

static void parse(Parser *parser) {
    if (parser->token->type == TOKEN_EOF)
        return;

    if (parser->token->type == TOKEN_NEWLINE) {
        step(parser, true);
        parse(parser);
        return;
    }

    if (parser->token->type == TOKEN_INT) {
        parse_rule(parser);
        return;
    }

    const Token *next = peek(parser, 1);

    if (next->type == TOKEN_EQUAL) {
        parse_constant(parser);
        return;
    }

    log_error("Invalid statement '%s' (%s)", parser->token->value, token_type_to_string(parser->token->type));
    step_to_next_line(parser);
}

Blueprint parse_blueprint(char *filename) {
    Blueprint bp = (Blueprint){ .rule_count = 0, .constant_count = 0 };
    Parser prs = create_parser(filename, &bp);

    while (prs.token->type != TOKEN_EOF)
        parse(&prs);

    delete_parser(&prs);
    return bp;
}