#ifndef BLUEPRINT_H
#define BLUEPRINT_H

#define MAX_RULES 32
#define MAX_CONSTANTS 64

#include <stdio.h>

typedef struct {
    size_t order;
    char *command;
} Rule;

typedef struct {
    char *name;
    char *value;
} Constant;

typedef struct {
    Rule rules[MAX_RULES];
    size_t rule_count;
    Constant constants[MAX_CONSTANTS];
    size_t constant_count;
} Blueprint;

void delete_blueprint(Blueprint *blueprint);
void push_rule(size_t order, char *command, Blueprint *blueprint);
void push_constant(char *name, char *value, Blueprint *blueprint);
Rule *find_rule(const size_t order, Blueprint *blueprint);
Constant *find_constant(const char *name, Blueprint *blueprint);

void dump_rules(const Blueprint *blueprint);
void dump_constants(const Blueprint *blueprint);

#endif