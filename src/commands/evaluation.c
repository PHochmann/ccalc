#include <stdio.h>
#include <string.h>

#include "evaluation.h"
#include "util.h"
#include "arith_context.h"
#include "../engine/console_util.h"
#include "../engine/tree_to_string.h"

void evaluation_init()
{

}

bool evaluation_check(__attribute__((unused)) char *input)
{
    return true;
}

void evaluation_exec(ParsingContext *ctx, char *input)
{
    Node *res;
    
    if (parse_input_wrapper(ctx, input, true, &res, true, true, true))
    {
        if (g_debug)
        {
            print_tree_visual(ctx, res);
            char inlined_tree[MAX_LINE_LENGTH];
            size_t size = tree_inline(ctx, res, inlined_tree, MAX_LINE_LENGTH, true);
            indicate_abbreviation(inlined_tree, size);
            printf("= %s\n", inlined_tree);
        }
        
        double eval = arith_eval(res);
        char result_str[ctx->min_str_len];
        ctx->to_string((void*)(&eval), result_str, ctx->min_str_len);

        if (!g_silent)
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
