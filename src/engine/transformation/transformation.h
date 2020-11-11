#pragma once
#include "../tree/node.h"
#include "matching.h"

void transform_by_matching(size_t num_free_vars,
    const char * const * free_vars,
    const Matching *matching,
    Node **to_transform);
