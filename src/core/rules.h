#pragma once
#include "../parsing/context.h"
#include "../parsing/parser.h"
#include "../transformation/matching.h"
#include "../transformation/rewrite_rule.h"

extern char *reduction_string;
extern char *derivation_string;
extern char *normal_form_string;
extern char *simplification_string;
extern char *pretty_string;

bool parse_ruleset_from_string(char *string, ParsingContext *ctx, MappingFilter default_filter, Ruleset *out_ruleset);
