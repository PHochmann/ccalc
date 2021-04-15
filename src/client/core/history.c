#include "../../util/vector.h"
#include "history.h"

#define HISTORY_STARTSIZE 8

Vector ans_vec; // Results of last evaluations (doubles)

void init_history()
{
    ans_vec = vec_create(sizeof(double), HISTORY_STARTSIZE);
}

void unload_history()
{
    vec_destroy(&ans_vec);
}

/*
Params
    index: 0 -> last evaluation, 1 -> second last evaluation etc.
*/
bool history_get(size_t index, double *out)
{
    if (vec_count(&ans_vec) <= index) return false;
    *out = *(double*)vec_get(&ans_vec, vec_count(&ans_vec) - 1 - index);
    return true;
}

void history_add(double value)
{
    VEC_PUSH_ELEM(&ans_vec, double, value);
}
