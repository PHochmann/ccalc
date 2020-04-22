#pragma once
#include <stdbool.h>
#include "../tree/node.h"

void init_history();
void unload_history();
void core_update_history(ConstantType value);
bool core_replace_history(Node **tree);
