# mp.h

A single header library for math expression parsing.

## Quick start

To build and run the examples provided run the following

```bash
$ make
```

## Usage

The library consists on two base components: the tokenizer and the parser.

After these two have processed the input expression a parse tree will be
created. For evaluating this parse tree, an interpreter and a basic virtual
machine have been implemented in this library.

In the simplified API of the library the compilation approach for the virtual
machine is used by default since it is preferred for the computational speed
advantage.

Examples can be found in the examples folder. A simple REPL and an example of
the usage of the simplified API can be found in the root directory of the
repository.

## Example

Here is the content of examples/example.c:

```c
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
```
