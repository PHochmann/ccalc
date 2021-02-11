#pragma once
#include <stdbool.h>
#include <sys/types.h>
#include "../../engine/tree/node.h"
#include "../../engine/tree/tree_util.h"

bool simplification_is_initialized();
ssize_t init_simplification(const char *file);
void unload_simplification();
ListenerError simplify(Node **tree, const Node **errnode);
