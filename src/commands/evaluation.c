#include <stdio.h>
#include <string.h>

#include "arith_context.h"
#include "arith_rules.h"
#include "evaluation.h"
#include "core.h"
#include "console_util.h"
#include "assignments.h"

#include "../engine/string_util.h"

void evaluation_init()
{
    g_ans = NULL;
}

bool evaluation_check(__attribute__((unused)) char *input)
{
    return true;
}

/*
Summary: The evaluation command is executed when input is no other command (hence last in command array at core.c)
*/
void evaluation_exec(ParsingContext *ctx, char *input)
{
    Node *res;
    
    if (parse_input_wrapper(ctx, input, &res, true, true, true))
    {
        // Show AST and string when debug=true
        if (g_debug)
        {
            printf("= ");
            print_tree_inlined(ctx, res, true);
            printf("\n");
        }
        
        double eval = arith_eval(res);
        char result_str[ctx->recomm_str_len];
        ctx->to_string((void*)(&eval), ctx->recomm_str_len, result_str);

        if (g_interactive)
        {
            printf("= %s\n", result_str);
        }
        else
        {
            printf("%s\n", result_str);
        }
        
        if (g_ans != NULL) free_tree(g_ans);
        g_ans = res;
    }
}
