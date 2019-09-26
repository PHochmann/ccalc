#include <stdio.h>
#include <string.h>

#include "cmd_evaluation.h"
#include "core.h"
#include "../console_util.h"
#include "../string_util.h"
#include "../parsing/node.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"

void cmd_evaluation_init() { }

bool cmd_evaluation_check(__attribute__((unused)) char *input)
{
    return true;
}

/*
Summary: The evaluation command is executed when input is no other command (hence last in command array at core.c)
*/
void cmd_evaluation_exec(ParsingContext *ctx, char *input)
{
    Node *tree;
    
    if (parse_input_console(ctx, input, "Error: %s\n", &tree, true, true))
    {        
        double result = arith_eval(tree);
        char result_str[ctx->recommended_str_len];
        ctx->to_string((void*)(&result), ctx->recommended_str_len, result_str);
        printf(g_interactive ? "= %s\n" : "%s\n", result_str);
    }
}
