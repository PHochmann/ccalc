#include <stdio.h>
#include <string.h>

#include "cmd_evaluation.h"
#include "core.h"
#include "console_util.h"

#include "../arith_context.h"
#include "../arith_rules.h"
#include "../string_util.h"

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
    
    if (parse_input_wrapper(ctx, input, &tree, true, true, true))
    {
        if (g_debug)
        {
            printf("= ");
            print_tree_inlined(ctx, tree, true);
            printf("\n");
        }
        
        double result = arith_eval(tree);
        char result_str[ctx->recomm_str_len];
        ctx->to_string((void*)(&result), ctx->recomm_str_len, result_str);
        printf(g_interactive ? "= %s\n" : "%s\n", result_str);
        if (g_ans != NULL) free_tree(g_ans);
        g_ans = tree;
    }
}
