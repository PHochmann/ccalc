#include <string.h>

#include "cmd_clear.h"
#include "../util/string_util.h"
#include "../util/console_util.h"
#include "../util/table.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"

#define COMMAND "table "
#define MAX_INLINED_LENGTH 100

bool cmd_table_check(char *input)
{
    return begins_with(COMMAND, input);
}

void print_current(Node *expr, char *var, double value)
{
    Node *current_expr = tree_copy(expr);
    Node *current_val = malloc_constant_node(value);
    replace_variable_nodes(&current_expr, current_val, var);
    add_cell(CONSTANT_TYPE_FMT, value);
    add_cell(CONSTANT_TYPE_FMT, arith_eval(current_expr));
    next_row();
    free_tree(current_expr);
    free_tree(current_val);
}

void cmd_table_exec(char *input)
{
    input += strlen(COMMAND);
    char *args[4];
    size_t num_args = split(input, args, 3, ";", ";", ";");

    if (num_args != 4)
    {
        printf("Table Error: Invalid syntax. Type 'help' for usage.\n");
        return;
    }

    Node *expr = NULL;
    Node *start = NULL;
    Node *end = NULL;
    Node *step = NULL;

    if (!parse_input_from_console(args[0], "Error in expression: %s\n", &expr, false)) return;

    char *variables[count_variables(expr)];
    if (list_variables(expr, variables) != 1)
    {
        printf("Expression contains none or more than one variable\n");
        goto exit;
    }

    if (!parse_input_from_console(args[1], "Error in start: %s\n", &start, false)
        || !parse_input_from_console(args[2], "Error in end: %s\n", &end, false)
        || !parse_input_from_console(args[3], "Error in step: %s\n", &step, false))
    {
        goto exit;
    }

    if (count_variables(start) > 0
        || count_variables(end) > 0
        || count_variables(step) > 0)
    {
        printf("Start, end and step must be constant\n");
        goto exit;
    }

    double start_val = arith_eval(start);
    double end_val = arith_eval(end);
    double step_val = arith_eval(step);

    if (step_val == 0
        || (start_val < end_val && step_val < 0)
        || (start_val > end_val && step_val > 0))
    {
        printf("Endless loop\n");
        goto exit;
    }

    // Print table

    add_cell("%s", variables[0]);

    char inlined_expr[MAX_INLINED_LENGTH];
    tree_inline(expr, inlined_expr, 100, false);
    add_cell_buffer(inlined_expr);

    next_row();

    if (start_val < end_val)
    {
        for (; start_val <= end_val; start_val += step_val)
        {
            print_current(expr, variables[0], start_val);
        }
    }
    else
    {
        for (; start_val >= end_val; start_val += step_val)
        {
            print_current(expr, variables[0], start_val);
        }
    }

    print_table(true);
    reset_table();

    exit:
    free_tree(expr);
    free_tree(start);
    free_tree(end);
    free_tree(step);
}
