#define MP_IMPLEMENTATION
#include "mp.h"

#include <time.h>

long benchmark_vm(const char *expression, size_t count)
{
    struct timespec start, end;

    MP_Env *mp = mp_init(expression);
    if (mp == NULL) {
        fprintf(stderr, "ERROR\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < count; ++i) {
        mp_evaluate(mp);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    long delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec -
            start.tv_nsec) / 1000;
    long ms = delta_us / 1000;

    mp_free(mp);

    return ms;
}

long benchmark_in(const char *expression, size_t count)
{
    struct timespec start, end;

    MP_Token_List token_list = {0};
    MP_Arena arena = {0};
    MP_Parse_Tree parse_tree = {0};

    MP_Result tr = mp_tokenize(&token_list, expression);
    if (tr.error) {
        mp_da_free(&token_list);
        mp_arena_free(&arena);
        return -1;
    }

    MP_Result pr = mp_parse(&arena, &parse_tree, token_list);
    if (pr.error) {
        mp_da_free(&token_list);
        mp_arena_free(&arena);
        return -1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < count; ++i) {
        mp_interpret(parse_tree);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    long delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec -
            start.tv_nsec) / 1000;
    long ms = delta_us / 1000;

    mp_da_free(&token_list);
    mp_arena_free(&arena);

    return ms;
}

int main(void)
{
    const int count = 1000*1000;
    const char *expr = "(12+45)*(78-34)/(9^3)+((56*(23+67)-(89/7))^2)-(15*(3+5)/(2^4))+((8-4)*(6+2)/(10-3))+(100/(25-5)*(3^2)-(7+2)*(5-1)+(4*(9+1)/(2^3)))-((11+22)*(33-44)/(55+66)+(77^2)-(88/(4+4))*(3+5)+(2*(6-1)^3))";

    long vm_time = benchmark_vm(expr, count);
    printf("vm(%d): %ld ms\n", count, vm_time);

    long in_time = benchmark_in(expr, count);
    printf("in(%d): %ld ms\n", count, in_time);

    return 0;
}
