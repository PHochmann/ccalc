#include <stdio.h>
#include <string.h>

#include "cmd_help.h"
#include "../string_util.h"
#include "../tree/operator.h"
#include "../tree/tree_to_string.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"
#include "../table/table.h"

#define COMMAND "help"

static char *VERSION = "1.4.7";

static char *INFOBOX_FMT =
    " Scientific calculator in which you can define your own functions and constants \n"
    "Version %s (c) 2019, Philipp Hochmann\n"
    "https://github.com/PhilippHochmann/Calculator";

static char *COMMAND_TABLE[7][2] = {
    { "<func|const> = <after>",                  "Adds new function or constant." },
    { "table <expr> ; <from> ; <to> ; <step>  \n"
      "   [fold <expr> ; <init>]",               "Prints table of values and optionally folds them.\n   In fold expression, 'x' is replaced with the intermediate result (init in first step),\n   'y' is replaced with the current value. Result is stored in 'ans'." },
    { "load <path>",                             "Loads file as if its content had been typed in." },
    { "debug <expr>",                            "Visually prints abstract syntax tree of expression." },
    { "help",                                    "Lists available commands and operators." },
    { "clear [last]",                            "Clears all or last user-defined functions and constants." },
    { "quit",                                    "Closes calculator." }
};

static const size_t BASIC_IND =  1; // Index of first basic operator ($x before, should no be shown)
static const size_t TRIG_IND  = 19; // Index of first trigonometric function
static const size_t MISC_IND  = 31; // Index of first misc. function
static const size_t CONST_IND = 48; // Index of first constant

bool cmd_help_check(char *input)
{
    return strcmp(COMMAND, input) == 0;
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

void cmd_help_exec(char __attribute__((unused)) *input)
{
    Table table = get_empty_table();
    add_cell_fmt(&table, get_settings_align(ALIGN_CENTER), INFOBOX_FMT, VERSION);
    make_boxed(&table, BORDER_SINGLE);
    print_table(&table);
    free_table(&table);
    printf("\n");

    table = get_empty_table();
    add_from_array(&table, 2, 7,
        (TextAlignment[]){ ALIGN_LEFT, ALIGN_LEFT },
        (char**)COMMAND_TABLE);
    print_table(&table);
    free_table(&table);

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
        table = get_empty_table();
        for (size_t i = ARITH_NUM_RULES; i < g_num_rules; i++)
        {
            char inlined_before[sizeof_tree_to_string(g_rules[i].before, true)];
            unsafe_tree_to_string(g_rules[i].before, inlined_before, true);
            add_cell_fmt(&table, get_settings_align(ALIGN_LEFT), "%s", inlined_before);

            add_cell(&table, get_settings_align(ALIGN_LEFT), " = ");

            char inlined_after[sizeof_tree_to_string(g_rules[i].after, true)];
            unsafe_tree_to_string(g_rules[i].after, inlined_after, true);
            add_cell_fmt(&table, get_settings_align(ALIGN_LEFT), "%s", inlined_after);

            next_row(&table);
        }
        print_table(&table);
        free_table(&table);
        printf("\n");
    }
    else
    {
        printf("\n\n");
    }
}
