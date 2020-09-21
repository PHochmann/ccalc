#include <stdio.h>
#include "console_util.h"
#include "alloc_wrappers.h"

#define ERROR_MESSAGE "ccalc out of heap memory - terminating\n"

void *malloc_wrapper(size_t size)
{
    void *res = malloc(size);
    if (size != 0 && res == NULL)
    {
        software_defect(ERROR_MESSAGE);
    }
    return res;
}

void *realloc_wrapper(void *ptr, size_t size)
{
    void *res = realloc(ptr, size);
    if (size != 0 && res == NULL)
    {
        software_defect(ERROR_MESSAGE);
    }
    return res;
}

void *calloc_wrapper(size_t n, size_t elem_size)
{
    void *res = calloc(n, elem_size);
    if (n != 0 && elem_size != 0 && res == NULL)
    {
        software_defect(ERROR_MESSAGE);
    }
    return res;
}
