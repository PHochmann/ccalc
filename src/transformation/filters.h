#pragma once
#include <stdbool.h>
#include "matching.h"

// Special rule prefixes:
#define MATCHING_CONST_PREFIX        'c'
#define MATCHING_CONST_OR_VAR_PREFIX 'b'
#define MATCHING_OP_OR_VAR_PREFIX    'd'
#define MATCHING_OP_PREFIX           'o'
#define MATCHING_LITERAL_VAR_PREFIX  'l'

bool prefix_filter(char *var, NodeList nodes);
