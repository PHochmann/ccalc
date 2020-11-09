#pragma once
#include "../parsing/context.h"
#include "../parsing/parser.h"
#include "../transformation/matching.h"
#include "../transformation/rewrite_rule.h"

#define NUM_RULESETS 7
extern char *g_rulestrings[NUM_RULESETS];

bool parse_rule(char *string, ParsingContext *ctx, MappingFilter default_filter, Vector *ruleset);
bool parse_ruleset_from_string(char *string, ParsingContext *ctx, MappingFilter default_filter, Ruleset *out_ruleset);
