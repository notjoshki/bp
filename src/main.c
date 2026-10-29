#include "blueprint.h"
#include "parser.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define OPT_QUIET 0x01

static void usage(const char *program) {
    printf("Usage: %s [options...] [filename]\n"
           "Options:\n"
           "    --help        Show this information\n"
           "    -q            Hide rule commands\n"
           , program);
}

static void insertion_sort(size_t *data, const size_t count) {
    for (size_t i = 1; i < count; i++) {
        size_t key = data[i];
        int j = (int)i - 1;

        while (j >= 0 && data[j] > key) {
            data[j + 1] = data[j];
            j = j - 1;
        }

        data[j + 1] = key;
    }
}

int main(int argc, char **argv) {
    char *filename = "bp.bp";
    size_t options = 0;

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--help") == 0) {
                usage(argv[0]);
                return EXIT_SUCCESS;
            }

            if (strcmp(argv[i], "-q") == 0)
                options |= OPT_QUIET;
            else if (i != argc - 1)
                log_error("Unknown option '%s'", argv[i]);
        }

        if (errors_logged() > 0)
            return EXIT_FAILURE;

        filename = argv[argc - 1];
    }

    Blueprint bp = parse_blueprint(filename);

#ifndef NDEBUG
    dump_rules(&bp);
    dump_constants(&bp);
#endif

    if (errors_logged() > 0) {
        fprintf(stderr, "Aborting...\n");
        delete_blueprint(&bp);
        return EXIT_FAILURE;
    }

    if (bp.rule_count == 0) {
        delete_blueprint(&bp);
        return EXIT_SUCCESS;
    }

    size_t orders[bp.rule_count];

    for (size_t i = 0; i < bp.rule_count; i++)
        orders[i] = bp.rules[i].order;

    insertion_sort(orders, bp.rule_count);

    const bool quiet = options & OPT_QUIET;

    for (size_t i = 0; i < bp.rule_count; i++) {
        const Rule *rule = find_rule(orders[i], &bp);

        if (!quiet)
            printf("%zu: %s\n", rule->order, rule->command);

        if (system(rule->command) != 0)
            fprintf(stderr, "Error: Rule %zu failed\n", rule->order);
    }

    delete_blueprint(&bp);
    return EXIT_SUCCESS;
}