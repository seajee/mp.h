#include <stdio.h>
#include <stdlib.h>

#include "interpreter.h"
#include "util.h"

int main(void)
{
    const char *expression = "(6 * 4) - 23 + (-3) * ((2 * -3) * 0.3) / 2";

    Arena arena = {0};
    Token_List token_list = {0};
    Parse_Tree parse_tree = {0};

    Result tr = tokenize(&token_list, expression);
    if (tr.error) {
        printf("%s\n", expression);
        printf("%*s^\n", (int)tr.error_position, "");
        printf("%ld: ERROR: %s\n", tr.error_position + 1,
               error_to_string(tr.error_type));
        goto cleanup;
    }

    Result pr = parse(&arena, &parse_tree, token_list);
    if (pr.error) {
        printf("%s\n", expression);
        printf("%*s^\n", (int)pr.error_position, "");
        printf("%ld: ERROR: %s\n", pr.error_position + 1,
               error_to_string(pr.error_type));
        goto cleanup;
    }

    Result r = interpret(parse_tree);
    if (r.error) {
        printf("ERROR: %s\n", error_to_string(r.error_type));
        goto cleanup;
    }

    printf("%s = %f\n", expression, r.value);

cleanup:
    da_free(&token_list);
    arena_free(&arena);

    return EXIT_SUCCESS;
}
