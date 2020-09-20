#include <string.h>
#include "alloc_wrappers.h"
#include "vector.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Fraction of original size that is added, e.g. 0.5 -> 150%
#define VECTOR_GROWTHFACTOR 0.5

void vec_trim(Vector *vec)
{
    vec->buffer_size = vec->elem_count + 1;
    vec->buffer = realloc_wrapper(vec->buffer, vec->elem_size * vec->buffer_size);
}

void vec_ensure_size(Vector *vec, size_t needed_size)
{
    while (needed_size > vec->buffer_size)
    {
        vec->buffer_size += MAX(1, (size_t)(vec->buffer_size * VECTOR_GROWTHFACTOR));
    }
    vec->buffer = realloc_wrapper(vec->buffer, vec->elem_size * vec->buffer_size);
}

Vector vec_create(size_t elem_size, size_t start_size)
{
    return (Vector){
        .elem_size   = elem_size,
        .elem_count  = 0,
        .buffer_size = start_size,
        .buffer      = malloc_wrapper(elem_size * start_size)
    };
}

void vec_clear(Vector *vec)
{
    vec->elem_count = 0;
}

void vec_destroy(Vector *vec)
{
    free(vec->buffer);
}

void *vec_get(Vector *vec, size_t index)
{
    return (char*)vec->buffer + vec->elem_size * index;
}

void *vec_push(Vector *vec, void *elem)
{
    return vec_push_many(vec, 1, elem);
}

/*
Summary: To push a literal fast, for convenience use VEC_PUSH_LITERAL-Macro
*/
void *vec_push_empty(Vector *vec)
{
    vec_ensure_size(vec, vec->elem_count + 1);
    return vec_get(vec, vec->elem_count++);
}

void *vec_push_many(Vector *vec, size_t num, void *elems)
{
    vec_ensure_size(vec, vec->elem_count + num);
    void *first = (char*)vec->buffer + vec->elem_size * vec->elem_count;
    memcpy(first, elems, num * vec->elem_size);
    vec->elem_count += num;
    return first;
}

void *vec_pop(Vector *vec)
{
    if (vec->elem_count == 0) return NULL;
    vec->elem_count--;
    return vec_get(vec, vec->elem_count);
}

void *vec_peek(Vector *vec)
{
    if (vec->elem_count == 0) return NULL;
    return vec_get(vec, vec->elem_count - 1);
}

size_t vec_count(Vector *vec)
{
    return vec->elem_count;
}

// Iterator implementation:

static void *get_next(Iterator *iterator)
{
    VectorIterator *it = (VectorIterator*)iterator;
    it->curr_index++;
    if ((size_t)it->curr_index < vec_count(it->vector))
    {
        return vec_get(it->vector, it->curr_index);
    }
    else
    {
        return NULL;
    }
}

static void reset(Iterator *iterator)
{
    ((VectorIterator*)iterator)->curr_index = -1;
}

VectorIterator vec_get_iterator(Vector *vec)
{
    return (VectorIterator){
        .base = (Iterator){ .get_next = get_next, .reset = reset },
        .vector = vec,
        .curr_index = -1
    };
}
