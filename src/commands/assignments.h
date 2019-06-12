#pragma once
#include "../engine/context.h"
#include "../engine/rule.h"

#define NUM_MAX_RULES 10

RewriteRule g_rules[NUM_MAX_RULES];
size_t g_num_rules;

void definition_init();
bool definition_check(char *input);
void definition_exec(ParsingContext *ctx, char *input);
void rule_init();
bool rule_check(char *input);
void rule_exec(ParsingContext *ctx, char *input);
