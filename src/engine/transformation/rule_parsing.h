#pragma once
#include <stdio.h>
#include <stdbool.h>

#include "../../util/vector.h"
#include "../parsing/context.h"
#include "rewrite_rule.h"

bool parse_pattern(const char *string,
    const ParsingContext *ctx,
    Pattern *out_pattern);

bool parse_rule(const char *string,
    const ParsingContext *ctx,
    RewriteRule *out_rule);

ssize_t parse_rulesets_from_file(FILE *file,
    const ParsingContext *ctx,
    size_t max_rulesets,
    Vector *out_rulesets);
