#include "iterator.h"

void *iterator_get_next(Iterator *iterator)
{
    return iterator->get_next(iterator);
}

void iterator_reset(Iterator *iterator)
{
    iterator->reset(iterator);
}
