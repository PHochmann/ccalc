#include <stdio.h>
#include <string.h>

#include "cmd_help.h"
#include "core.h"
#include "../util/console_util.h"

#include "../parsing/operator.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"

#define COL_RESET "\033[0m"
// White background, black foreground
#define OP_COLOR "\x1B[47m" "\x1B[22;30m"

static const size_t BASIC_IND     =  2; // Index of first basic operator ($x before, should no be shown)
static const size_t TRIG_IND      = 19; // Index of first trigonometric function
static const size_t MISC_FUNC_IND = 31; // Index of first misc. function
static const size_t CONSTANTS_IND = 45; // Index of first constant

bool cmd_help_check(char *input)
{
    return strcmp(input, "help") == 0;
}

void print_op(Operator *op)
{
    printf(OP_COLOR);
    switch (op->placement)
    {
        case OP_PLACE_PREFIX:
            if (op->arity != 0)
            {
                printf("%sx", op->name);
            }
            else
            {
                printf("%s", op->name);
            }
            break;
            
        case OP_PLACE_INFIX:
            if (strlen(op->name) == 1)
            {
                printf("x%sy", op->name);
            }
            else
            {
                printf("x %s y", op->name);
            }
            break;
            
        case OP_PLACE_POSTFIX:
            printf("x%s", op->name);
            break;
            
        case OP_PLACE_FUNCTION:
            switch (op->arity)
            {
                case 0:
                    printf("%s", op->name);
                    break;
                case OP_DYNAMIC_ARITY:
                    printf("%s(*)", op->name);
                    break;
                default:
                    printf("%s(%lu)", op->name, op->arity);
            }
    }
    
    printf(COL_RESET " ");
}

void cmd_help_exec(__attribute__((unused)) char *input)
{
    printf("Calculator %s (c) 2019, Philipp Hochmann\n"
           "Commands:\n"
           "\t<func|const> = <after>\n"
           "\tload <path>\n"
           "\tdebug <expr>\n"
           "\ttable <expr> ; <from> ; <to> ; <step>\n",
        VERSION);

    printf("\nBasic operators:\n");
    for (size_t i = BASIC_IND; i < TRIG_IND; i++)
    {
        print_op(&g_ctx->operators[i]);
    }

    printf("\nTrigonometric functions:\n");
    for (size_t i = TRIG_IND; i < MISC_FUNC_IND; i++)
    {
        print_op(&g_ctx->operators[i]);
    }

    printf("\nMiscellaneous functions:\n");
    for (size_t i = MISC_FUNC_IND; i < CONSTANTS_IND; i++)
    {
        print_op(&g_ctx->operators[i]);
    }

    printf("\nConstants:\n");
    for (size_t i = CONSTANTS_IND; i < ARITH_NUM_OPS; i++)
    {
        print_op(&g_ctx->operators[i]);
    }

    // Print user-defined functions if there are any
    if (g_num_rules > ARITH_NUM_RULES)
    {
        printf("\nUser-defined functions and constants:");
        for (size_t i = ARITH_NUM_RULES; i < g_num_rules; i++)
        {
            printf("\n");
            print_tree_inlined(g_rules[i].before, true);
            printf(" = ");
            print_tree_inlined(g_rules[i].after, true);
        }
    }

    printf("\n\n");
}
