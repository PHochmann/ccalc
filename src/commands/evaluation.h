#pragma once
#include "../engine/context.h"
#include "../engine/node.h"

Node *g_ans; // Constant node that contains result of last evaluation

void evaluation_init();
bool evaluation_check(char *input);
void evaluation_exec(ParsingContext *ctx, char *input);
