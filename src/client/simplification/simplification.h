#pragma once
#include <stdbool.h>
#include <sys/types.h>
#include "../../engine/tree/node.h"

ssize_t init_simplification(char *file);
void unload_simplification();
bool core_simplify(Node **tree);
