#include <string.h>
#include "stack.h"

Stack stack_create(size_t elem_size, size_t start_size)
{
    return (Stack){
        .elem_size = elem_size,
        .elem_count = 0,
        .buffer_size = start_size,
        .buffer = malloc(elem_size * start_size)
    };
}

void stack_destroy(Stack *stack)
{
    free(stack->buffer);
}

void *stack_get(Stack *stack, size_t index)
{
    return (char*)stack->buffer + stack->elem_size * index;
}

void *stack_set(Stack *stack, size_t index, void *elem)
{
    while (index >= stack->buffer_size)
    {
        stack->buffer_size <<= 1;
    }
    stack->buffer = realloc(stack->buffer, stack->elem_size * stack->buffer_size);
    memcpy((char*)stack->buffer + stack->elem_size * index, elem, stack->elem_size);
    return (void*)((char*)stack->buffer + stack->elem_size * index);
}

void *stack_push(Stack *stack, void *elem)
{
    return stack_set(stack, stack->elem_count, elem);
}

void *stack_pop(Stack *stack)
{
    stack->elem_count--;
    return stack_get(stack, stack->elem_count);
}

void *stack_peek(Stack *stack)
{
    return stack_get(stack, stack->elem_count - 1);
}

size_t stack_count(Stack *stack)
{
    return stack->elem_count;
}
