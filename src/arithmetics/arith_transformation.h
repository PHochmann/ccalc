/*
Summary: Handles post-processing of parsed trees, i.e. substitution of ans and rule appliances
*/

#pragma once
#include "../tree/context.h"
#include "../matching/rewrite_rule.h"

void arith_init_transformation();
void arith_unload_transformation();

bool arith_can_add_rule();
bool arith_add_rule(RewriteRule rule);
bool arith_pop_rule();
size_t arith_get_num_userdefined();
RewriteRule *arith_get_userdefined(size_t index);

void arith_update_ans(ConstantType value);
bool arith_transform_input(bool all_rules, Node **tree);
