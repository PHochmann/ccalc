#include <stdio.h>
#include <string.h>

#include "cmd_help.h"
#include "core.h"
#include "../parsing/operator.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"
#include "../util/string_util.h"
#include "../util/console_util.h"
#include "../util/table.h"

static const size_t BASIC_IND     =  2; // Index of first basic operator ($x before, should no be shown)
static const size_t TRIG_IND      = 19; // Index of first trigonometric function
static const size_t MISC_FUNC_IND = 31; // Index of first misc. function
static const size_t CONSTANTS_IND = 46; // Index of first constant

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

char *commands[8][2] = {
    { "<func|const> = <after>  ",                "Adds new function or constant." },
    { "table <expr> ; <from> ; <to> ; <step>  ", "Prints table of values." },
    { "load <path>  ",                           "Loads file as if its content had been typed in." },
    { "debug <expr>  ",                          "Visually prints abstract syntax tree of expression." },
    { "help  ",                                  "Lists all available commands and operators." },
    { "clear  ",                                 "Clears all user-defined functions." },
    { "quit  ",                                  "Closes calculator." }
};

void cmd_help_exec(__attribute__((unused)) char *input)
{
    printf("Calculator %s (c) 2019, Philipp Hochmann\n", VERSION);
    add_cells_from_array(0, 1, 2, 7, commands, TEXTPOS_LEFT_ALIGNED, TEXTPOS_LEFT_ALIGNED);
    print_table(false);
    reset_table();

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
            char inlined[100];
            tree_inline(g_rules[i].before, inlined, 100, false);
            add_cell(TEXTPOS_LEFT_ALIGNED, "%s", inlined);
            add_cell(TEXTPOS_LEFT_ALIGNED, " = ");
            tree_inline(g_rules[i].after, inlined, 100, false);
            add_cell(TEXTPOS_LEFT_ALIGNED, "%s", inlined);
            next_row();
        }
        print_table(false);
        reset_table();
    }

    printf("\n\n");
}
