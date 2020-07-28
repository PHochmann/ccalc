#include <stdio.h>
#include <string.h>

#include "cmd_help.h"
#include "../util/string_util.h"
#include "../tree/operator.h"
#include "../tree/tree_to_string.h"
#include "../core/arith_context.h"
#include "../table/table.h"
#include "../core/evaluation.h"

#define HELP      "help"
#define HELP_OPS  "help operators"
#define SHORT_HELP_CODE 1
#define OPS_HELP_CODE   2

#define VERSION "1.5.5"

#ifdef DEBUG
    #define RELEASE VERSION " DEBUG"
#else
    #define RELEASE VERSION
#endif

#define TTY_WIDTH 80

static char *INFOBOX_FMT =
    "ccalc %s (c) 2020 Philipp Hochmann\n"
    " Scientific calculator in which you can define new functions and constants \n";

static char *COMMAND_TABLE[7][2] = {
    { "<func|const> = <after>",                  "Adds or redefines function or constant." },
    { "table <expr> ; <from> ; <to> ; <step>  \n"
      "   [fold <expr> ; <init>]",               "Prints table of values." },
    { "load <path>",                             "Executes commands in file." },
    { "clear [<func>]",                          "Clears all or one function or constant." },
    { "quit",                                    "Closes application." }
};

static char *OP_DESCRIPTIONS[52] = {
    " Addition ",
    " Subtraction ",
    " Multiplication ",
    " Division ",
    " Exponentiation ",
    " Binomial coefficient ",
    " Modulo operator ",
    " Identity ",
    " Negation ",
    " Factorial ",
    " Division by 100 ",
    " Natural exponential function ",
    " nth root of x ",
    " Square root ",
    " Logarithm to base n ",
    " Natural logarithm ",
    " Binary logarithm ",
    " Logarithm to base 10 ",
    " Sine ",
    " Cosine ",
    " Tangent ",
    " Inverse sine ",
    " Inverse cosine ",
    " Inverse tangens ",
    " Hyperbolic sine ",
    " Hyperbolic cosine ",
    " Hyperbolic tangent ",
    " Inverse hyperbolic sine ",
    " Inverse hyperbolic cosine ",
    " Inverse hyperbolic tangent ",
    " Maximum ",
    " Minimum ",
    " Absolute value ",
    " Round up to nearest integer ",
    " Round down to nearest integer ",
    " Round to nearest integer ",
    " Round towards zero to nearest integer ",
    " Fractional part of x ",
    " Sign of x (-1, 0, 1) ",
    " Sum of all operands ",
    " Product of all operands ",
    " Arithmetic mean of all operands ",
    " Greatest common divisor ",
    " Least common multiple ",
    " Random integer between min and max (exclusive) ",
    " Fibonacci sequence ",
    " Gamma function ",
    " Archimedes' constant ",
    " Euler's number ",
    " Golden ratio ",
    " Speed of light [m/s] ",
    " Speed of sound in air at 20 °C [m/s] "
};

static const size_t BASIC_IND =  4; // Index of first basic operator ($x, x@y, deriv before, should no be shown)
static const size_t TRIG_IND  = 22; // Index of first trigonometric function
static const size_t MISC_IND  = 34; // Index of first misc. function
static const size_t CONST_IND = 51; // Index of first constant
static const size_t LAST_IND  = 56; // Index of last constant

int cmd_help_check(char *input)
{
    if (strcmp(HELP, input) == 0) return SHORT_HELP_CODE;
    if (strcmp(HELP_OPS, input) == 0) return OPS_HELP_CODE;
    return false;
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
    ListNode *curr = list_get_node(&g_ctx->op_list, start);
    while (curr != NULL && start != end)
    {
        Operator *op = (Operator*)curr->data;
        remaining_width -= print_op(op);
        if (remaining_width <= 0)
        {
            printf("\n");
            remaining_width = TTY_WIDTH;
        }
        start++;
        curr = curr->next;
    }
}

void print_op_table(OpPlacement place, bool assoc, bool precedence, bool arity, bool value)
{
    Table table = get_empty_table();
    set_default_alignments(&table, 2, (TextAlignment[]){ ALIGN_LEFT, ALIGN_NUMBERS });
    override_alignment_of_row(&table, ALIGN_LEFT);
    add_cell(&table, " Name ");
    if (precedence) add_cell(&table, " Precedence ");
    if (assoc) add_cell(&table, " Assoc. ");
    if (arity) add_cell(&table, " Arity ");
    if (value) add_cell(&table, " Value ");
    add_cell(&table, " Description ");
    next_row(&table);

    ListNode *curr = list_get_node(&g_ctx->op_list, BASIC_IND);
    size_t index = BASIC_IND;
    while (curr != NULL && index < LAST_IND)
    {
        Operator *op = (Operator*)curr->data;

        if (op->placement == place && (place != OP_PLACE_FUNCTION || (value == (op->arity == 0))))
        {
            add_cell_fmt(&table, " %s ", op->name);

            if (precedence)
            {
                add_cell_fmt(&table, " %d ", op->precedence);
            }

            if (assoc)
            {
                switch (op->assoc)
                {
                    case OP_ASSOC_LEFT:
                        add_cell(&table, " Left ");
                        break;
                    case OP_ASSOC_RIGHT:
                        add_cell(&table, " Right ");
                }
            }

            if (arity)
            {
                if (op->arity != OP_DYNAMIC_ARITY)
                {
                    add_cell_fmt(&table, " %d ", op->arity);
                }
                else
                {
                    add_cell(&table, " * ");
                }
            }

            if (value)
            {
                double const_val;
                op_evaluate(op, 0, NULL, &const_val);
                override_alignment(&table, ALIGN_NUMBERS);
                add_cell_fmt(&table, " " CONSTANT_TYPE_FMT " ", const_val);
            }
            
            add_cell(&table, OP_DESCRIPTIONS[index - BASIC_IND]);
            next_row(&table);
        }
        index++;
        curr = curr->next;
    }

    print_table(&table);
    free_table(&table);
    printf("\n");
}

void print_op_tables()
{
    print_op_table(OP_PLACE_INFIX, true, true, false, false);
    print_op_table(OP_PLACE_PREFIX, false, true, false, false);
    print_op_table(OP_PLACE_POSTFIX, false, true, false, false);
    print_op_table(OP_PLACE_FUNCTION, false, false, true, false);
    print_op_table(OP_PLACE_FUNCTION, false, false, false, true);
}

void print_short_help()
{
    Table table = get_empty_table();
    add_cell_fmt(&table, INFOBOX_FMT, RELEASE);
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
    if (list_count(&g_composite_functions) > 0)
    {
        printf("\nUser-defined functions and constants:\n");
        table = get_empty_table();
        ListNode *curr = g_composite_functions.first;
        while (curr != NULL)
        {
            RewriteRule *rule = (RewriteRule*)curr->data;
            add_cell_gc(&table, tree_to_str(rule->before, true));
            add_cell(&table, " = ");
            add_cell_gc(&table, tree_to_str(rule->after, true));
            next_row(&table);
            curr = curr->next;
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

bool cmd_help_exec(__attribute__((unused)) char *input, int code)
{
    if (code == SHORT_HELP_CODE)
    {
        print_short_help();
    }
    else
    {
        print_op_tables();
    }
    return true;
}
