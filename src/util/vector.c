#include <string.h>
#include "vector.h"

#include <stdio.h>
void *realloc_wrapped(void *ptr, size_t n)
{
    void *res = realloc(ptr, n);
    if (res == NULL) printf("ALARM!\n");
    return res;
}

Vector vec_create(size_t elem_size, size_t start_size)
{
    if (start_size == 0)
    {
        start_size = 1;
    }
    return (Vector){
        .elem_size = elem_size,
        .elem_count = 0,
        .buffer_size = start_size,
        .buffer = malloc(elem_size * start_size)
    };
}

void vec_destroy(Vector *vec)
{
    free(vec->buffer);
}

void *vec_get(Vector *vec, size_t index)
{
    return (char*)vec->buffer + vec->elem_size * index;
}

void *vec_set(Vector *vec, size_t index, void *elem)
{
    while (index >= vec->buffer_size)
    {
        vec->buffer_size <<= 1;
    }
    vec->buffer = realloc_wrapped(vec->buffer, vec->elem_size * vec->buffer_size);
    memcpy((char*)vec->buffer + vec->elem_size * index, elem, vec->elem_size);

    return (void*)((char*)vec->buffer + vec->elem_size * index);
}

void *vec_push(Vector *vec, void *elem)
{
    return vec_set(vec, vec->elem_count++, elem);
}

void vec_push_many(Vector *vec, size_t num, void *elem)
{
    while (vec->elem_count + num >= vec->buffer_size)
    {
        vec->buffer_size <<= 1;
    }
    vec->buffer = realloc_wrapped(vec->buffer, vec->elem_size * vec->buffer_size);
    memcpy((char*)vec->buffer + vec->elem_size * vec->elem_count, elem, num * vec->elem_size);
    vec->elem_count += num;
}

void *vec_pop(Vector *vec)
{
    vec->elem_count--;
    return vec_get(vec, vec->elem_count);
}

void *vec_peek(Vector *vec)
{
    return vec_get(vec, vec->elem_count - 1);
}

size_t vec_count(Vector *vec)
{
    return vec->elem_count;
}
