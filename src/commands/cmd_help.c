#include <stdio.h>
#include <string.h>

#include "cmd_help.h"
#include "../string_util.h"
#include "../tree/operator.h"
#include "../tree/tree_to_string.h"
#include "../core/arith_context.h"
#include "../table/table.h"

#define COMMAND   "help"
#define TTY_WIDTH 80

static char *VERSION = "1.5.0";

static char *INFOBOX_FMT =
    "Calculator %s (c) 2020, Philipp Hochmann\n"
    " Scientific calculator in which you can define new functions and constants \n"
    "https://github.com/PhilippHochmann/Calculator";

static char *COMMAND_TABLE[7][2] = {
    { "<func|const> = <after>",                  "Adds or redefines function or constant." },
    { "table <expr> ; <from> ; <to> ; <step>  \n"
      "   [fold <expr> ; <init>]",               "Prints table of values." },
    { "load <path>",                             "Executes commands in file." },
    { "clear [last]",                            "Clears functions and constants." },
    { "quit",                                    "Closes application." }
};

static const size_t BASIC_IND =  2; // Index of first basic operator ($x and x@y before, should no be shown)
static const size_t TRIG_IND  = 20; // Index of first trigonometric function
static const size_t MISC_IND  = 32; // Index of first misc. function
static const size_t CONST_IND = 49; // Index of first constant
static const size_t LAST_IND  = 54; // Index of last constant

bool cmd_help_check(char *input)
{
    return strcmp(COMMAND, input) == 0;
}

// Returns number of characters printed
int print_op(Operator *op)
{
    printf(OP_COLOR);
    int res = 0;
    switch (op->placement)
    {
        case OP_PLACE_PREFIX:
            if (op->arity != 0)
            {
                res = printf("%sx", op->name);
            }
            else
            {
                res = printf("%s", op->name);
            }
            break;
            
        case OP_PLACE_INFIX:
            if (is_letter(op->name[0]))
            {
                res = printf("x %s y", op->name);
            }
            else
            {
                res = printf("x%sy", op->name);
            }
            break;
            
        case OP_PLACE_POSTFIX:
            res = printf("x%s", op->name);
            break;
            
        case OP_PLACE_FUNCTION:
            switch (op->arity)
            {
                case 0:
                    res = printf("%s", op->name);
                    break;
                case OP_DYNAMIC_ARITY:
                    res = printf("%s(*)", op->name);
                    break;
                default:
                    res = printf("%s(%zu)", op->name, op->arity);
            }
    }
    printf(COL_RESET " ");
    return res + 1;
}

void print_ops_between(size_t start, size_t end)
{
    int remaining_width = TTY_WIDTH;
    for (size_t i = start; i < end; i++)
    {
        remaining_width -= print_op(&g_ctx->operators[i]);
        if (i + 1 < end)
        {
            if (remaining_width - (int)strlen(g_ctx->operators[i + 1].name) - 4 < 0)
            {
                printf("\n");
                remaining_width = TTY_WIDTH;
            }
        }
    }
}

bool cmd_help_exec(char __attribute__((unused)) *input)
{
    Table table = get_empty_table();
    add_cell_fmt(&table, INFOBOX_FMT, VERSION);
    next_row(&table);
    make_boxed(&table, BORDER_SINGLE);
    set_default_alignments(&table, 1, (TextAlignment[]){ ALIGN_CENTER });
    print_table(&table);
    free_table(&table);
    printf("\n");

    table = get_empty_table();
    add_cells_from_array(&table, 2, 7, (char**)COMMAND_TABLE);
    print_table(&table);
    free_table(&table);

    printf("\nBasic operators:\n");
    print_ops_between(BASIC_IND, TRIG_IND);
    printf("\nTrigonometric functions:\n");
    print_ops_between(TRIG_IND, MISC_IND);
    printf("\nMiscellaneous functions:\n");
    print_ops_between(MISC_IND, CONST_IND);
    printf("\nConstants:\n");
    print_ops_between(CONST_IND, LAST_IND);

    // Print user-defined functions if there are any
    if (get_num_composite_functions() > 0)
    {
        printf("\nUser-defined functions and constants:\n");
        table = get_empty_table();
        for (size_t i = 0; i < get_num_composite_functions(); i++)
        {
            RewriteRule *rule = get_composite_function(i);

            char inlined_before[sizeof_tree_to_string(rule->before, true)];
            unsafe_tree_to_string(rule->before, inlined_before, true);
            add_cell_fmt(&table, "%s", inlined_before);

            add_cell(&table, " = ");

            char inlined_after[sizeof_tree_to_string(rule->after, true)];
            unsafe_tree_to_string(rule->after, inlined_after, true);
            add_cell_fmt(&table, "%s", inlined_after);

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

    return true;
}
