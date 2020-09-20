#pragma once
#include <stdlib.h>

typedef struct Iterator
{
    void *(*get_next)(struct Iterator *iterator);
    void (*reset)(struct Iterator *iterator);
} Iterator;

void *iterator_get_next(Iterator *iterator);
void iterator_reset(Iterator *iterator);
