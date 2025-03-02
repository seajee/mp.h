#ifndef UTIL_H_
#define UTIL_H_

#include <assert.h>
#include <stdlib.h>

#define DA_INITIAL_CAPACITY 256

#define da_append(da, item)                                                            \
    do {                                                                               \
        if ((da)->count >= (da)->capacity) {                                           \
            (da)->capacity = (da)->capacity == 0                                       \
                ? DA_INITIAL_CAPACITY : (da)->capacity * 2;                            \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Out of memory");                            \
        }                                                                              \
        (da)->items[(da)->count++] = (item);                                           \
    } while (0)

#define da_free(da)         \
    do {                    \
        free((da)->items);  \
        (da)->items = NULL; \
        (da)->count = 0;    \
        (da)->capacity = 0; \
    } while (0)

#define da_reset(da)     \
    do {                 \
        (da)->count = 0; \
    } while (0)

#define UNREACHABLE() do { assert(0 && "UNREACHABLE"); } while (0)

#define TODO() do { assert(0 && "TODO"); } while (0)

#endif // UTIL_H_
