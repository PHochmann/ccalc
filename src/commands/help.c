#include <stdio.h>
#include <string.h>

#include "help.h"
#include "core.h"
#include "command.h"
#include "util.h"
#include "../engine/operator.h"

void help_init()
{

}

bool help_check(char *input)
{
    return strcmp(input, "help") == 0;
}

void help_exec(ParsingContext *ctx, __attribute__((unused)) char *input)
{
#ifdef DEBUG
    printf("Calculator %s Debug build (c) 2019, Philipp Hochmann\n", VERSION);
#else
    printf("Calculator %s (c) 2019, Philipp Hochmann\n", VERSION);
#endif
    printf("Commands: debug, help, rules, <function> := <after>, <before> -> <after>, load <path>\n");

    for (size_t i = 0; i < ctx->num_ops; i++)
    {
        printf(OP_COLOR);
        switch (ctx->operators[i].placement)
        {
            case OP_PLACE_PREFIX:
                if (ctx->operators[i].arity != 0)
                {
                    printf("%sx", ctx->operators[i].name);
                }
                else
                {
                    printf("%s", ctx->operators[i].name);
                }
                break;
                
            case OP_PLACE_INFIX:
                if (strlen(ctx->operators[i].name) == 1)
                {
                    printf("x%sy", ctx->operators[i].name);
                }
                else
                {
                    printf("x %s y", ctx->operators[i].name);
                }
                break;
                
            case OP_PLACE_POSTFIX:
                printf("x%s", ctx->operators[i].name);
                break;
                
            case OP_PLACE_FUNCTION:
                if (ctx->operators[i].arity != DYNAMIC_ARITY)
                {
                    printf("%s(%d)", ctx->operators[i].name, ctx->operators[i].arity);
                }
                else
                {
                    printf("%s(*)", ctx->operators[i].name);
                }
                break;
        }

        if (g_debug && ctx->operators[i].arity > 0 && ctx->operators[i].placement != OP_PLACE_FUNCTION)
        {
            if (ctx->operators[i].placement == OP_PLACE_INFIX)
            {
                char assoc = '?';
                switch (ctx->operators[i].assoc)
                {
                    case OP_ASSOC_BOTH:
                        assoc = 'B';
                        break;
                    case OP_ASSOC_LEFT:
                        assoc = 'L';
                        break;
                    case OP_ASSOC_RIGHT:
                        assoc = 'R';
                        break;
                }

                printf(COL_RESET "%d,%c ", ctx->operators[i].precedence, assoc);
            }
            else
            {
                printf(COL_RESET "%d ", ctx->operators[i].precedence);
            }
            
        }
        else
        {
            printf(COL_RESET " ");
        }
        
    }
    printf("\n(%zu available operators)\n", ctx->num_ops);
}
