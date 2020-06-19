#pragma once
#include <stdbool.h>
#include "../tree/node.h"

void init_simplification();
void unload_simplification();
bool core_simplify(Node **tree, bool experimental_simplification, bool elim_constant_subtrees);
