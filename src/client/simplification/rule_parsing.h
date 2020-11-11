#pragma once
#include <stdbool.h>

#include "../../engine/util/vector.h"
#include "../../engine/parsing/context.h"

bool parse_rule(char *string, ParsingContext *ctx, Vector *ruleset);
bool parse_ruleset_from_string(const char *string, ParsingContext *ctx, Vector *out_ruleset);
