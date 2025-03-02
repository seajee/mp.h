#ifndef ARENA_H_
#define ARENA_H_

#include <stddef.h>

#define ARENA_DEFAULT_CAPACITY (8*1024)

typedef struct {
    size_t count;
    size_t capacity;
    void *data;
} Arena;

Arena arena_init(size_t capacity);
void *arena_alloc(Arena *arena, size_t size);
void arena_free(Arena *arena);
void arena_reset(Arena *arena);

#endif // ARENA_H_
