#include <stdlib.h>

typedef struct
{
    size_t elem_size;
    size_t elem_count;
    size_t buffer_size;
    void *buffer;
} Stack;

Stack stack_create(size_t elem_size, size_t start_size);
void stack_destroy(Stack *stack);

void *stack_get(Stack *stack, size_t index);
void *stack_set(Stack *stack, size_t index, void *elem);
void *stack_push(Stack *stack, void *elem);
void *stack_pop(Stack *stack);
void *stack_peek(Stack *stack);

size_t stack_count(Stack *stack);
