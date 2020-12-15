#include "../../util/vector.h"
#include "../../util/console_util.h"
#include "../../engine/transformation/matching.h"
#include "../../engine/parsing/parser.h"
#include "../../engine/tree/tree_util.h"

#include "history.h"
#include "arith_context.h"
#include "arith_evaluation.h"

Vector ans_vec; // Results of last evaluations (doubles)

void init_history()
{
    ans_vec = vec_create(sizeof(double), 5);
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
