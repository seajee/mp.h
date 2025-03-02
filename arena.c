#include "arena.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

Arena arena_init(size_t capacity)
{
    Arena arena = {0};
    arena.capacity = capacity;
    arena.count = 0;
    arena.data = malloc(capacity);
    assert(arena.data != NULL);

    return arena;
}

void *arena_alloc(Arena *arena, size_t size)
{
    if (arena->data == NULL) {
        *arena = arena_init(ARENA_DEFAULT_CAPACITY);
    }

    assert(arena->count + size <= arena->capacity);

    void *result = (uint8_t*)arena->data + arena->count;
    arena->count += size;

    return result;
}

void arena_free(Arena *arena)
{
    arena->count = 0;
    arena->capacity = 0;
    free(arena->data);
}

void arena_reset(Arena *arena)
{
    arena->count = 0;
}
