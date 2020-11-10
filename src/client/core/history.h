#pragma once
#include <stdbool.h>
#include "../../engine/tree/node.h"

void init_history();
void unload_history();
void core_add_history(double value);
bool core_replace_history(Node **tree);
