#pragma once
#include <stdio.h>
#include <stdbool.h>

#include "../util/vector.h"
#include "../parsing/context.h"
#include "rewrite_rule.h"

bool parse_rule(char *string, ParsingContext *main_ctx, ParsingContext *extended_ctx, RewriteRule *out_rule);
size_t parse_rulesets_from_file(FILE *file,
    ParsingContext *main_ctx,
    ParsingContext *extended_ctx,
    size_t max_rulesets,
    Vector *out_rulesets);
