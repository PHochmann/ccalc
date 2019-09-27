#pragma once
#include "../parsing/context.h"
#include "../matching/rewrite_rule.h"

#define ARITH_MAX_RULES 11
#define ARITH_NUM_RULES  1

size_t g_num_rules;
RewriteRule g_rules[ARITH_MAX_RULES];

void arith_reset_rules();
void arith_init_rules();
void transform_input(Node *tree, bool update_ans);
