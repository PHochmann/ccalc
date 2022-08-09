#include <stdio.h>
#include <string.h>

#include "../../util/string_util.h"
#include "../../util/console_util.h"
#include "../../engine/tree/operator.h"
#include "../../engine/tree/tree_to_string.h"
#include "../../table/table.h"
#include "../core/arith_context.h"
#include "../core/arith_evaluation.h"
#include "../version.h"
#include "cmd_help.h"

#define HELP            "help"
#define HELP_OPS        "help operators"
#define LICENSE_COMMAND "license"
#define SHORT_HELP_CODE 1
#define OPS_HELP_CODE   2
#define LICENSE_CODE    3
#define TTY_WIDTH       80

static const char *LICENSE =
    COPYRIGHT_NOTICE
    "Scientific calculator in which you can define new functions and constants\n"
    "https://github.com/PhilippHochmann/ccalc\n"
    "\n"
    "This program is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n"
    "\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program.  If not, see <https://www.gnu.org/licenses/>.\n";

#define NUM_COMMANDS 9
static const char *COMMAND_TABLE[NUM_COMMANDS][2] = {
    { "<func|const> = <after>",                  "Adds function or constant" },
    { "table <expr> ; <from> ; <to> ; <step>  \n"
      "   [fold <expr> ; <init>]",               "Prints table of values" },
    { "load [simplification] <path>",            "Executes commands or loads simplification ruleset in file" },
    { "clear [<func>]",                          "Clears all or one function or constant" },
    { "help [operators]",                        "Shows this message or a verbose list of all operators" },
    { "license",                                 "Shows information about ccalc's license" },
    { "quit",                                    "Closes application" }
};

static const char *OP_DESCRIPTIONS[NUM_ARITH_OPS] = {
    " Identity to parse rest of expression as if put in parentheses ",
    " History operator ",
    " Derivative shorthand ",
    " Derivative of expression with respect to variable ",
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
    " Median of all values ",
    " Greatest common divisor ",
    " Least common multiple ",
    " Random integer between min and max (exclusive) ",
    " Fibonacci sequence ",
    " Gamma function ",
    " Variance of a population ",
    " Archimedes' constant ",
    " Euler's number ",
    " Golden ratio ",
    " Speed of light [m/s] ",
    " Speed of sound in air at 20 °C [m/s] ",
    " Last result "
};

static const size_t PSEUDO_IND =  0; // Index of first pseudo operator ($, @, ', deriv)
static const size_t BASIC_IND  =  4; // Index of first basic operator
static const size_t TRIG_IND   = 22; // Index of first trigonometric function
static const size_t MISC_IND   = 34; // Index of first misc. function
static const size_t CONST_IND  = 52; // Index of first constant
static const size_t LAST_IND   = 58; // Index of last constant

int cmd_help_check(const char *input)
{
    if (strcmp(HELP, input) == 0) return SHORT_HELP_CODE;
    if (strcmp(HELP_OPS, input) == 0) return OPS_HELP_CODE;
    if (strcmp(LICENSE_COMMAND, input) == 0) return LICENSE_CODE;
    return 0;
}

// Returns number of characters printed
static int print_op(Operator *op)
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

static void print_ops_between(size_t start, size_t end)
{
    int remaining_width = TTY_WIDTH;
    ListNode *curr = list_get_node(&g_ctx->op_list, start);
    while (curr != NULL && start != end)
    {
        Operator *op = (Operator*)listnode_get_data(curr);
        remaining_width -= print_op(op);
        if (remaining_width <= 0)
        {
            printf("\n");
            remaining_width = TTY_WIDTH;
        }
        start++;
        curr = listnode_get_next(curr);
    }
}

static void print_op_table(OpPlacement place, bool assoc, bool precedence, bool arity, bool value)
{
    Table *table = get_empty_table();
    set_default_alignments(table, 2, (TableHAlign[]){ H_ALIGN_LEFT, H_ALIGN_RIGHT }, NULL);
    override_horizontal_alignment_of_row(table, H_ALIGN_LEFT);
    add_cell(table, " Name ");
    if (precedence) add_cell(table, " Precedence ");
    if (assoc) add_cell(table, " Assoc. ");
    if (arity) add_cell(table, " Arity ");
    if (value) add_cell(table, " Value ");
    add_cell(table, " Description ");
    next_row(table);

    ListNode *curr = list_get_node(&g_ctx->op_list, PSEUDO_IND);
    size_t index = PSEUDO_IND;
    while (curr != NULL && index < LAST_IND)
    {
        Operator *op = (Operator*)listnode_get_data(curr);

        if (op->placement == place && (place != OP_PLACE_FUNCTION || (value == (op->arity == 0))))
        {
            add_cell_fmt(table, " %s ", op->name);

            if (precedence)
            {
                add_cell_fmt(table, " %d ", op->precedence);
            }

            if (assoc)
            {
                switch (op->assoc)
                {
                    case OP_ASSOC_LEFT:
                        add_cell(table, " Left ");
                        break;
                    case OP_ASSOC_RIGHT:
                        add_cell(table, " Right ");
                }
            }

            if (arity)
            {
                if (op->arity != OP_DYNAMIC_ARITY)
                {
                    add_cell_fmt(table, " %zu ", op->arity);
                }
                else
                {
                    add_cell(table, " * ");
                }
            }

            if (value)
            {
                double const_val;
                override_horizontal_alignment(table, H_ALIGN_RIGHT);
                if (arith_op_evaluate(op, 0, NULL, &const_val) == LISTENERERR_SUCCESS)
                {
                    add_cell_fmt(table, " " CONSTANT_TYPE_FMT " ", const_val);
                }
                else
                {
                    add_empty_cell(table);
                }
            }
            
            add_cell(table, OP_DESCRIPTIONS[index]);
            next_row(table);
        }
        index++;
        curr = listnode_get_next(curr);
    }

    print_table(table);
    free_table(table);
}

void print_short_help()
{
    Table *table = get_empty_table();
    add_cells_from_array(table, 2, NUM_COMMANDS, (const char**)COMMAND_TABLE);
    print_table(table);
    free_table(table);

    printf("\nPseudo operators:\n");
    print_ops_between(PSEUDO_IND, BASIC_IND);
    printf("\nBasic operators:\n");
    print_ops_between(BASIC_IND, TRIG_IND);
    printf("\nTrigonometric functions:\n");
    print_ops_between(TRIG_IND, MISC_IND);
    printf("\nMiscellaneous functions:\n");
    print_ops_between(MISC_IND, CONST_IND);
    printf("\nConstants:\n");
    print_ops_between(CONST_IND, LAST_IND);
    printf("\n");

    // Print user-defined functions if there are any
    if (list_count(g_composite_functions) > 0)
    {
        printf("User-defined functions and constants:\n");
        table = get_empty_table();
        ListNode *curr = g_composite_functions->first;
        while (curr != NULL)
        {
            RewriteRule *rule = (RewriteRule*)listnode_get_data(curr);
            add_cell_gc(table, tree_to_str(rule->pattern.pattern, true));
            add_cell(table, " = ");
            add_cell_gc(table, tree_to_str(rule->after, true));
            next_row(table);
            curr = listnode_get_next(curr);
        }
        print_table(table);
        free_table(table);
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
        if (code == LICENSE_CODE)
        {
            // GitHub CI reports formats-security error, albeit COPYRIGHT_NOTICE being a string literal
            fputs(LICENSE, stdout);
        }
        else
        {
            printf("Infix operators:\n");
            print_op_table(OP_PLACE_INFIX, true, true, false, false);
            printf("\nPrefix operators:\n");
            print_op_table(OP_PLACE_PREFIX, false, true, false, false);
            printf("\nPostfix operators:\n");
            print_op_table(OP_PLACE_POSTFIX, false, true, false, false);
            printf("\nFunctions:\n");
            print_op_table(OP_PLACE_FUNCTION, false, false, true, false);
            printf("\nConstants:\n");
            print_op_table(OP_PLACE_FUNCTION, false, false, false, true);
        }
    }
    return true;
}
