#include <stdio.h>
#include <string.h>

#include "cmd_help.h"

#include "../parsing/operator.h"
#include "../parsing/tokenizer.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"
#include "../util/string_util.h"
#include "../util/console_util.h"
#include "../util/table.h"

#define MAX_INLINED_LENGTH 100
#define VERSION "1.4.3"

static const size_t BASIC_IND =  1; // Index of first basic operator ($x before, should no be shown)
static const size_t TRIG_IND  = 19; // Index of first trigonometric function
static const size_t MISC_IND  = 31; // Index of first misc. function
static const size_t CONST_IND = 46; // Index of first constant

static char *command_descriptions[7][2] = {
    { "<func|const> = <after>",                  "Adds new function or constant." },
    { "table <expr> ; <from> ; <to> ; <step>  ", "Prints table of values." },
    { "load <path>",                             "Loads file as if its content had been typed in." },
    { "debug <expr>",                            "Visually prints abstract syntax tree of expression." },
    { "help",                                    "Lists all available commands and operators." },
    { "clear",                                   "Clears all user-defined functions." },
    { "quit",                                    "Closes calculator." }
};

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
            if (is_letter(op->name[0]))
            {
                printf("x %s y", op->name);
            }
            else
            {
                printf("x%sy", op->name);
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
                    printf("%s(%zu)", op->name, op->arity);
            }
    }
    printf(COL_RESET " ");
}

void cmd_help_exec(__attribute__((unused)) char *input)
{
    printf("Calculator %s (c) 2019, Philipp Hochmann\n", VERSION);

    Table table = get_table();
    add_cells_from_array(&table, 0, 0, 2, 7, command_descriptions, TEXTPOS_LEFT, TEXTPOS_LEFT);
    print_table(&table, false);
    reset_table(&table);

    printf("\nBasic operators:\n");
    for (size_t i = BASIC_IND; i < TRIG_IND; i++)
    {
        print_op(&g_ctx->operators[i]);
    }

    printf("\nTrigonometric functions:\n");
    for (size_t i = TRIG_IND; i < MISC_IND; i++)
    {
        print_op(&g_ctx->operators[i]);
    }

    printf("\nMiscellaneous functions:\n");
    for (size_t i = MISC_IND; i < CONST_IND; i++)
    {
        print_op(&g_ctx->operators[i]);
    }

    printf("\nConstants:\n");
    for (size_t i = CONST_IND; i < ARITH_NUM_OPS; i++)
    {
        print_op(&g_ctx->operators[i]);
    }

    // Print user-defined functions if there are any
    if (g_num_rules > ARITH_NUM_RULES)
    {
        printf("\nUser-defined functions and constants:\n");
        for (size_t i = ARITH_NUM_RULES; i < g_num_rules; i++)
        {
            char inlined[MAX_INLINED_LENGTH];
            tree_to_string(g_rules[i].before, inlined, MAX_INLINED_LENGTH, true);
            add_cell_fmt(&table, TEXTPOS_LEFT, inlined);
            add_cell(&table, TEXTPOS_LEFT, " = ");
            tree_to_string(g_rules[i].after, inlined, MAX_INLINED_LENGTH, true);
            add_cell_fmt(&table, TEXTPOS_LEFT, "%s", inlined);
            next_row(&table);
        }
        print_table(&table, false);
        reset_table(&table);
        printf("\n");
    }
    else
    {
        printf("\n\n");
    }
}
