#pragma once
#include <stdbool.h>
#include "../../engine/tree/node.h"

void init_history();
void unload_history();
void history_add(double value);
bool history_get(size_t index, double *out);
