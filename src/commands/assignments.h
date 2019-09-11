#pragma once
#include "../engine/context.h"
#include "../engine/rule.h"

void definition_init();
bool definition_check(char *input);
void definition_exec(ParsingContext *ctx, char *input);

void rule_init();
bool rule_check(char *input);
void rule_exec(ParsingContext *ctx, char *input);
