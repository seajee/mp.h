#define MP_IMPLEMENTATION
#include "mp.h"

int main(void)
{
    const char *expr = "x * (2 + x) / 2";

    MP_Env *parser = mp_init(expr);
    if (parser == NULL) {
        fprintf(stderr, "ERROR: Invalid expression\n");
        mp_free(parser);
        return EXIT_FAILURE;
    }

    mp_variable(parser, 'x', 10.0);
    MP_Result result = mp_evaluate(parser);
    if (result.error) {
        fprintf(stderr, "ERROR: Could not evaluate expression\n");
        mp_free(parser);
        return EXIT_FAILURE;
    }

    printf("%f\n", result.value);

    mp_free(parser);

    return EXIT_SUCCESS;
}
