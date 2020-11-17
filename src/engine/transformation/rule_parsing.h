#pragma once
#include <stdio.h>
#include <stdbool.h>

#include "../util/vector.h"
#include "../parsing/context.h"
#include "rewrite_rule.h"

bool parse_pattern(const char *string,
    const ParsingContext *main_ctx,
    const ParsingContext *extended_ctx,
    Pattern *out_pattern);

bool parse_rule(const char *string,
    const ParsingContext *main_ctx,
    const ParsingContext *extended_ctx,
    RewriteRule *out_rule);

size_t parse_rulesets_from_file(FILE *file,
    const ParsingContext *main_ctx,
    const ParsingContext *extended_ctx,
    size_t max_rulesets,
    Vector *out_rulesets);
