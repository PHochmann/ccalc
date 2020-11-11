#pragma once
#include "../tree/node.h"
#include "matching.h"

void transform_by_matching(const Pattern *pattern, const Matching *matching, Node *to_transform);
