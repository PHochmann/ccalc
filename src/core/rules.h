#pragma once
#include "../parsing/context.h"
#include "../parsing/parser.h"
#include "../transformation/matching.h"
#include "../transformation/rewrite_rule.h"

extern char *g_reduction_string;
extern char *g_derivation_string;
extern char *g_normal_form_string;
extern char *g_simplification_string;
extern char *g_fold_string;
extern char *g_pretty_string;

bool parse_rule(char *string, ParsingContext *ctx, MappingFilter default_filter, Vector *ruleset);
bool parse_ruleset_from_string(char *string, ParsingContext *ctx, MappingFilter default_filter, Ruleset *out_ruleset);
