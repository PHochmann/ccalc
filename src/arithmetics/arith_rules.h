#pragma once
#include "../parsing/context.h"
#include "../matching/rewrite_rule.h"

#define ARITH_MAX_RULES 14
#define ARITH_NUM_RULES  4

size_t g_num_rules;
RewriteRule g_rules[ARITH_MAX_RULES];

void arith_reset_rules();
void arith_unload_rules();
void arith_init_rules();
void update_ans(ConstantType value);
void transform_input(Node **tree);
