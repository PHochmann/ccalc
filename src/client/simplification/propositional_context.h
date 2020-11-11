#pragma once
#include "../../engine/parsing/context.h"

#define g_propositional_ctx (&__g_propositional_ctx)
extern ParsingContext __g_propositional_ctx;

void init_propositional_ctx();
void unload_propositional_ctx();
