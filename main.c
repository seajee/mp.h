#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"
#include "util.h"
#include "vm.h"

#define INPUT_BUFFER_CAPACITY 512

int main(void)
{
    bool quit = false;
    char input[INPUT_BUFFER_CAPACITY];
    bool debug = false;

    Arena arena = {0};
    Token_List token_list = {0};
    Parse_Tree parse_tree = {0};

    printf("Type `help` for more information\n");

    while (!quit) {
        /* Input */

        printf("> ");
        if (fgets(input, INPUT_BUFFER_CAPACITY, stdin) == NULL) {
            goto cleanup;
        }

        if (strcmp(input, "help\n") == 0) {
            printf("Usage:\n");
            printf("  Type an expression or a command to use the application\n");
            printf("  Supported operations: + - * /\n");
            printf("  Example:\n");
            printf("    2 * (4.3 / 3.1) - 8\n");
            printf("\n");
            printf("Commands:\n");
            printf("  debug:    Toggle debug information\n");
            printf("  exit:     Quit the application\n");
            printf("  help:     Show this text\n");
            continue;
        } else if (strcmp(input, "exit\n") == 0) {
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

        /* Tokenize and parse input */

        Result tr = tokenize(&token_list, input);
        if (tr.error) {
            printf("  %*s^\n", (int)tr.error_position, "");
            printf("%ld: ERROR: tokenizer: %s\n", tr.error_position + 1,
                   error_to_string(tr.error_type));
            goto reset;
        }

        if (debug) {
            printf("\n===== Token_List =====\n");
            print_token_list(token_list);
            printf("\n");
        }

        Result pr = parse(&arena, &parse_tree, token_list);
        if (pr.error) {
            printf("  %*s^\n", (int)pr.error_position, "");
            printf("%ld: ERROR: parser: %s\n", pr.error_position + 1,
                   error_to_string(pr.error_type));
            goto reset;
        }

        if (debug) {
            printf("\n===== Parse_Tree =====\n");
            print_parse_tree(parse_tree);
            printf("\n");
        }

        /* Compile input down to Program and execute it */

        double result = 0.0;

        Program program = {0};
        if (!program_compile(&program, parse_tree)) {
            printf("ERROR: Could not compile program\n");
            da_free(&program);
            goto reset;
        }

        if (debug) {
            printf("\n===== Program (%ld bytes) =====\n", program.count);
            print_program(program);
            printf("\n");
        }

        Vm vm = vm_init(program);

        if (!vm_run(&vm)) {
            printf("ERROR: Error while executing VM program\n");
            vm_free(&vm);
            da_free(&program);
            goto reset;
        }

        result = vm_result(&vm);
        vm_free(&vm);
        da_free(&program);

        printf("%.10g\n", result);
reset:
        arena_reset(&arena);
        da_reset(&token_list);
    }

cleanup:
    da_free(&token_list);
    arena_free(&arena);

    return EXIT_SUCCESS;
}
