#ifndef ARENA_H_
#define ARENA_H_

#include <stdint.h>

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

#ifdef ARENA_IMPLEMENTATION

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

#endif // ARENA_IMPLEMENTATION
