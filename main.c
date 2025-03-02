#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"
#include "util.h"

int main(void)
{
    Arena arena = {0};
    Token_List token_list = {0};
    Parse_Tree parse_tree = {0};

    bool quit = false;
    char input[512];
    bool debug = false;

    while (!quit) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            goto cleanup;
        }

        if (strcmp(input, "exit\n") == 0) {
            quit = true;
            goto cleanup;
        } else if (strcmp(input, "debug\n") == 0) {
            debug = !debug;
            if (debug)
                printf("on\n");
            else
                printf("off\n");

            continue;
        }

        Result tr = tokenize(&token_list, input);
        if (tr.error) {
            printf("  %*s^\n", (int)tr.error_position, "");
            printf("%ld: ERROR: %s\n", tr.error_position + 1,
                   error_to_string(tr.error_type));
            goto reset;
        }

        if (debug) {
            print_token_list(token_list);
        }

        Result pr = parse(&arena, &parse_tree, token_list);
        if (pr.error) {
            printf("  %*s^\n", (int)pr.error_position, "");
            printf("%ld: ERROR: %s\n", pr.error_position + 1,
                   error_to_string(pr.error_type));
            goto reset;
        }

        if (debug) {
            print_parse_tree(parse_tree);
        }

        Result r = interpret(parse_tree);
        if (r.error) {
            printf("ERROR: %s\n", error_to_string(r.error_type));
            goto reset;
        }

        printf("%f\n", r.value);
reset:
        arena_reset(&arena);
        da_reset(&token_list);
    }

cleanup:
    da_free(&token_list);
    arena_free(&arena);

    return EXIT_SUCCESS;
}
