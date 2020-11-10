#pragma once
#include "../../engine/parsing/context.h"

#define g_extended_ctx (&__g_extended_ctx)
extern ParsingContext __g_extended_ctx;

void init_extended_ctx();
void unload_extended_ctx();
