#define MP_IMPLEMENTATION
#include "mp.h"

int main(void)
{
    const char *expr = "(2 + 2) * 3";
    bool error = false;

    MP_Token_List token_list = {0};
    MP_Arena arena = {0};
    MP_Parse_Tree parse_tree = {0};

    MP_Result tr = mp_tokenize(&token_list, expr);
    if (tr.error) {
        fprintf(stderr, "ERROR: tokenizer: %s\n",
                mp_error_to_string(tr.error_type));
        error = true;
        goto cleanup;
    }

    MP_Result pr = mp_parse(&arena, &parse_tree, token_list);
    if (pr.error) {
        fprintf(stderr, "ERROR: parser: %s\n",
                mp_error_to_string(pr.error_type));
        error = true;
        goto cleanup;
    }

    MP_Result ir = mp_interpret(parse_tree);
    if (ir.error) {
        fprintf(stderr, "ERROR: interpreter: %s\n",
                mp_error_to_string(pr.error_type));
        error = true;
        goto cleanup;
    }

    printf("%f\n", ir.value);

cleanup:
    mp_da_free(&token_list);
    mp_arena_free(&arena);

    if (error) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
