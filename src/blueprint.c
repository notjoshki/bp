#include "blueprint.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void delete_blueprint(Blueprint *blueprint) {
    for (size_t i = 0; i < blueprint->rule_count; i++)
        free(blueprint->rules[i].command);

    for (size_t i = 0; i < blueprint->constant_count; i++) {
        free(blueprint->constants[i].name);
        free(blueprint->constants[i].value);
    }
}

void push_rule(size_t order, char *command, Blueprint *blueprint) {
    if (blueprint->rule_count + 1 > MAX_RULES) {
        log_error("Maximum rule count of %u exceeded", MAX_RULES);
        exit(EXIT_FAILURE);
    }

    blueprint->rules[blueprint->rule_count++] = (Rule){ .order = order, .command = command };
}

void push_constant(char *name, char *value, Blueprint *blueprint) {
    if (blueprint->constant_count + 1 > MAX_CONSTANTS) {
        log_error("Maximum constant count of %u exceeded", MAX_CONSTANTS);
        exit(EXIT_FAILURE);
    }

    blueprint->constants[blueprint->constant_count++] = (Constant){ .name = name, .value = value };
}

Rule *find_rule(const size_t order, Blueprint *blueprint) {
    for (size_t i = 0; i < blueprint->rule_count; i++) {
        Rule *rule = &blueprint->rules[i];

        if (rule->order == order)
            return rule;
    }

    return NULL;
}

Constant *find_constant(const char *name, Blueprint *blueprint) {
    for (size_t i = 0; i < blueprint->constant_count; i++) {
        Constant *constant = &blueprint->constants[i];

        if (strcmp(constant->name, name) == 0)
            return constant;
    }
    
    return NULL;
}

void dump_rules(const Blueprint *blueprint) {
    printf("--- DUMPING RULES ---\n");

    for (size_t i = 0; i < blueprint->rule_count; i++)
        printf("%zu: order: %zu, command: \"%s\"\n", i, blueprint->rules[i].order, blueprint->rules[i].command);

    printf("------\n\n");
}

void dump_constants(const Blueprint *blueprint) {
    printf("--- DUMPING CONSTANTS ---\n");

    for (size_t i = 0; i < blueprint->constant_count; i++)
        printf("%zu: name: \"%s\", value: \"%s\"\n", i, blueprint->constants[i].name, blueprint->constants[i].value);

    printf("------\n\n");
}