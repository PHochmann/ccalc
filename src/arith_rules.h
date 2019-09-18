#pragma once
#include "parsing/context.h"
#include "matching/rule.h"

#define ARITH_MAX_RULES 20

// Exported to let show_rules command know which rules to display
extern const size_t ARITH_NUM_PREDEFINED_RULES;

size_t g_num_rules;
RewriteRule g_rules[ARITH_MAX_RULES];

void arith_reset_rules();
void arith_init_rules(ParsingContext *ctx);
