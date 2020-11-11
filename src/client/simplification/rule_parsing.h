#pragma once
#include <stdbool.h>

#include "../../engine/util/vector.h"
#include "../../engine/parsing/context.h"

bool parse_rule(char *string, ParsingContext *main_ctx, ParsingContext *extended_ctx, RewriteRule *out_rule);
bool parse_ruleset_from_string(const char *string, ParsingContext *main_ctx, ParsingContext *extended_ctx, Vector *out_ruleset);
