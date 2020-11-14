#pragma once
#include <stdbool.h>
#include "../../engine/tree/node.h"

void init_simplification();
void unload_simplification();
bool core_simplify(Node **tree);
