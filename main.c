#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"
#include "util.h"
#include "vm.h"

int main(void)
{
    Arena arena = {0};
    Token_List token_list = {0};
    Parse_Tree parse_tree = {0};

    bool quit = false;
    char input[512];
    bool debug = false;
    bool compile = false;

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
        } else if (strcmp(input, "compile\n") == 0) {
            compile = !compile;
            if (compile)
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

        Result pr = parse(&arena, &parse_tree, token_list);
        if (pr.error) {
            printf("  %*s^\n", (int)pr.error_position, "");
            printf("%ld: ERROR: %s\n", pr.error_position + 1,
                   error_to_string(pr.error_type));
            goto reset;
        }

        if (debug) {
            printf("\n=== DEBUG ===\n");
            printf("Token list:\n");
            print_token_list(token_list);
            printf("\nParse tree:\n");
            print_parse_tree(parse_tree);
        }

        double result = 0.0;

        if (compile) {
            Program program = program_compile(parse_tree);

            if (debug) {
                printf("\nVM Program:\n");
                print_program(program);
            }

            Vm vm = vm_init(program);
            vm_run(&vm);
            result = vm_result(&vm);
            vm_free(&vm);
        } else {
            Result r = interpret(parse_tree);
            if (r.error) {
                printf("ERROR: %s\n", error_to_string(r.error_type));
                goto reset;
            }
            result = r.value;
        }

        if (debug)
            printf("=============\n\n");

        printf("%f\n", result);
reset:
        arena_reset(&arena);
        da_reset(&token_list);
    }

cleanup:
    da_free(&token_list);
    arena_free(&arena);

    return EXIT_SUCCESS;
}
