#include <stdio.h>
#include "string_builder.h"

StringBuilder strbuilder_create(size_t start_size)
{
    StringBuilder builder = vec_create(sizeof(char), start_size);
    *(char*)vec_push_empty(&builder) = '\0';
    return builder;
}

void strbuilder_reset(StringBuilder *builder)
{
    vec_reset(builder),
    *(char*)vec_push_empty(builder) = '\0';
}

void strbuilder_append(StringBuilder *builder, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vstrbuilder_append(builder, fmt, args);
    va_end(args);
}

void strbuilder_append_char(StringBuilder *builder, char c)
{
    VEC_SET_ELEM(builder, char, builder->elem_count - 1, c);
    VEC_PUSH_ELEM(builder, char, '\0');
}

/*void strbuilder_reverse(StringBuilder *builder)
{
    for (size_t i = 0; i < (vec_count(builder) - 1) / 2; i++)
    {
        char temp = VEC_GET_ELEM(builder, char, i);
        size_t partner = vec_count(builder) - 2 - i;
        VEC_SET_ELEM(builder, char, i, VEC_GET_ELEM(builder, char, partner));
        VEC_SET_ELEM(builder, char, partner, temp);
    }
}*/

void vstrbuilder_append(StringBuilder *builder, char *fmt, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    size_t appended_length = vsnprintf(vec_get(builder, vec_count(builder) - 1),
        builder->buffer_size - builder->elem_count + 1,
        fmt, args);

    if (vec_ensure_size(builder, vec_count(builder) + appended_length))
    {
        vsnprintf(vec_get(builder, vec_count(builder) - 1),
            builder->buffer_size - builder->elem_count + 1,
            fmt, args_copy);
    }
    builder->elem_count += appended_length;
    va_end(args_copy);
}

char *strbuilder_to_str(StringBuilder *builder)
{
    return builder->buffer;
}
