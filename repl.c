#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MP_IMPLEMENTATION
#include "mp.h"

#define INPUT_BUFFER_CAPACITY 512

int get_user_input(void); // -1: exit, 0: ok, 1: continue
void set_vars(MP_Env *mp, double vars[26]);

char input[INPUT_BUFFER_CAPACITY];
double vars[26];

int main(void)
{
    MP_Env *mp = NULL;

    printf("Type `help` for more information\n");

    while (true) {
        /* Read */
        int r = get_user_input();
        if (r == -1)
            break;
        if (r == 1)
            continue;

        /* Eval */
        mp = mp_init(input);
        set_vars(mp, vars);

        MP_Result result = mp_evaluate(mp);
        if (result.error) {
            fprintf(stderr, "TODO\n");
            mp_free(mp);
            mp = NULL;
            continue;
        }

        /* Print */
        printf("%.10g\n", result.value);
        mp_free(mp);
        mp = NULL;
    }

    mp_free(mp);

    return EXIT_SUCCESS;
}

int get_user_input(void)
{
    printf("> ");
    if (fgets(input, INPUT_BUFFER_CAPACITY, stdin) == NULL) {
        return -1;
    }

    if (strcmp(input, "help\n") == 0) {
        printf("Usage:\n");
        printf("  Type an expression or a command to use the application.\n");
        printf("  Supported operations: (+) (-) (*) (/) (^)\n");
        printf("  Example: 2 * (4.3 / 3.1) - 8\n");
        printf("\n");
        printf("Commands:\n");
        printf("  exit:     Quit the application\n");
        printf("  help:     Show this text\n");
        printf("  set:      Set a variable to a specific value (a - z)\n");
        return 1;

    } else if (strcmp(input, "exit\n") == 0) {
        return -1;

    } else if (strcmp(input, "set\n") == 0) {
        printf("var: ");
        if (fgets(input, INPUT_BUFFER_CAPACITY, stdin) == NULL)
            return -1;
        char var = input[0];

        if (var < 'a' || var > 'z') {
            printf("ERROR: Invalid variable name (a - z)\n");
            return 1;
        }

        printf("value: ");
        if (fgets(input, INPUT_BUFFER_CAPACITY, stdin) == NULL)
            return -1;
        char *end = NULL;
        double num = strtod(input, &end);
        if (input == end) {
            printf("ERROR: Expected a number\n");
            return 1;
        }

        vars[var - 'a'] = num;
        printf("%c = %.10g\n", var, num);

        return 1;
    }

    return 0;
}

void set_vars(MP_Env *mp, double vars[26])
{
    for (size_t i = 0; i < 26; ++i) {
        mp_variable(mp, i + 'a', vars[i]);
    }

    mp_variable(mp, 'p', MP_PI);
    mp_variable(mp, 'e', MP_E);
}
