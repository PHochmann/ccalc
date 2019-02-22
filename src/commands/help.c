#include <stdio.h>
#include <string.h>

#include "help.h"
#include "command.h"
#include "util.h"
#include "../engine/constants.h"
#include "../engine/operator.h"
#include "../engine/console_util.h"

char *placement_to_string(OpPlacement placement, Arity arity)
{
    if (placement == OP_PLACE_PREFIX && arity == 0)
    {
        return "Constant";
    }

    switch (placement)
    {
        case OP_PLACE_FUNCTION:
            return "Function";
        case OP_PLACE_INFIX:
            return "Infix";
        case OP_PLACE_POSTFIX:
            return "Postfix";
        case OP_PLACE_PREFIX:
            return "Prefix";
    }

    return "Undefined";
}

void table(ParsingContext *ctx)
{
    // Name | type | arity | precendece

    char *table[4 + ctx->num_ops * 4];

    table[0] = "Name";
    table[1] = "Type";
    table[2] = "Arity";
    table[3] = "Precedence";

    for (size_t i = 0; i < ctx->num_ops; i++)
    {
        Operator *op = &ctx->operators[i];

        table[4 + i * 4 + 0] = op->name;
        table[4 + i * 4 + 1] = placement_to_string(op->placement, op->arity);
        table[4 + i * 4 + 2] = "-1";
        table[4 + i * 4 + 3] = "-2";
    }

    print_table(ctx->num_ops + 1, 4, table, true);
}

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
    printf("Calculator %s Debug build (c) 2018-2019, Philipp Hochmann\n", VERSION);
#else
    printf("Calculator %s (c) 2018-2019, Philipp Hochmann\n", VERSION);
#endif
    printf("Commands: debug, help, <function> := <after>, <before> -> <after>\n");

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
            printf(COL_RESET "p:%d ", ctx->operators[i].precedence);
        }
        else
        {
            printf(COL_RESET " ");
        }
        
    }
    printf("\n(%zu available operators)\n", ctx->num_ops);

    if (g_debug) table(ctx);
}
