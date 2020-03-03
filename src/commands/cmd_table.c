#include <string.h>

#include "cmd_table.h"
#include "../console_util.h"
#include "../string_util.h"
#include "../tree/tree_to_string.h"
#include "../core/arith_context.h"
#include "../core/history.h"
#include "../core/evaluation.h"
#include "../table/table.h"

#define COMMAND "table "
#define FOLD_KEYWORD " fold "
#define FOLD_VAR_1 "x"
#define FOLD_VAR_2 "y"

bool cmd_table_check(char *input)
{
    return begins_with(COMMAND, input);
}

bool cmd_table_exec(char *input)
{
    input += strlen(COMMAND);
    char *args[6];
    size_t num_args = str_split(input, args, 5, ";", ";", ";", FOLD_KEYWORD, ";");

    if (num_args != 4 && num_args != 6)
    {
        printf("Error: Invalid syntax.\n");
        return false;
    }

    bool success = false;
    Node *expr = NULL;
    Node *start = NULL;
    Node *end = NULL;
    Node *step = NULL;
    Node *fold_expr = NULL;
    Node *fold_init = NULL;

    if (!core_parse_input(args[0], "Error in expression: %s.\n", true, &expr))
    {
        return false;
    }

    char *variables[count_variables(expr)];
    size_t num_vars = list_variables(expr, variables);
    if (num_vars > 1)
    {
        printf("Error: Expression contains more than one variable.\n");
        goto exit;
    }

    if (!core_parse_input(args[1], "Error in start: %s.\n", true, &start)
        || !core_parse_input(args[2], "Error in end: %s.\n", true, &end)
        || !core_parse_input(args[3], "Error in step: %s.\n", true, &step))
    {
        goto exit;
    }

    if (count_variables(start) > 0
        || count_variables(end) > 0
        || count_variables(step) > 0)
    {
        printf("Error: Start, end and step must be constant.\n");
        goto exit;
    }

    double start_val = arith_evaluate(start);
    double end_val = arith_evaluate(end);
    double step_val = arith_evaluate(step);

    if (step_val == 0)
    {
        printf("Error: Step must not be zero.\n");
        goto exit;
    }

    // Optionally: Parse part of command after "fold"
    double fold_val = 0;
    if (num_args == 6)
    {
        // Parse initial fold-value
        if (!core_parse_input(args[4], "Error in fold expression: %s.\n", true, &fold_expr)
            || !core_parse_input(args[5], "Error in initial fold value: %s.\n", true, &fold_init))
        {
            goto exit;
        }

        if (count_variables(fold_expr)
            - count_variable_nodes(fold_expr, FOLD_VAR_1)
            - count_variable_nodes(fold_expr, FOLD_VAR_2) != 0)
        {
            printf("Error: Fold expression must not contain any variables except '"
                FOLD_VAR_1 "' and '" FOLD_VAR_2 "'.\n");
            goto exit;
        }

        if (count_variables(fold_init) > 0)
        {
            printf("Error: Initial fold value must be constant.\n");
            goto exit;
        }

        fold_val = arith_evaluate(fold_init);
    }
    // - - - End of parsing of fold-construct

    // Adjust step direction to not have an endless loop
    if ((start_val < end_val && step_val < 0)
        || (start_val > end_val && step_val > 0))
    {
        step_val = -step_val;
    }

    Table table = get_empty_table();
    
    // Header
    if (g_interactive)
    {
        add_cell(&table, " # ");
        set_vline(&table, BORDER_SINGLE);
        if (num_vars != 0)
        {
            add_cell_fmt(&table, VAR_COLOR " %s " COL_RESET, variables[0]);
        }
        else
        {
            add_empty_cell(&table);
        }

        char inlined_expr[sizeof_tree_to_string(expr, true)];
        unsafe_tree_to_string(expr, inlined_expr, true);
        set_vline(&table, BORDER_SINGLE);
        add_cell_fmt(&table, " %s ", inlined_expr);
        override_alignment_of_row(&table, ALIGN_CENTER);
        next_row(&table);
        set_hline(&table, BORDER_SINGLE);
    }

    // Loop through all values and add them to table
    for (size_t i = 1; step_val > 0 ? start_val <= end_val : start_val >= end_val; i++)
    {
        Node *current_expr = tree_copy(expr);
        Node *current_val = malloc_constant_node(start_val);
        replace_variable_nodes(&current_expr, current_val, variables[0]);
        double result = arith_evaluate(current_expr);

        add_cell_fmt(&table, " %zu ", i);
        add_cell_fmt(&table, " " CONSTANT_TYPE_FMT " ", start_val);
        add_cell_fmt(&table, " " CONSTANT_TYPE_FMT " ", result);

        if (num_args == 6)
        {
            Node *current_fold = tree_copy(fold_expr);
            Node *current_fold_x = malloc_constant_node(fold_val);
            Node *current_fold_y = malloc_constant_node(result);
            replace_variable_nodes(&current_fold, current_fold_x, FOLD_VAR_1);
            replace_variable_nodes(&current_fold, current_fold_y, FOLD_VAR_2);
            fold_val = arith_evaluate(current_fold);
            free_tree(current_fold);
            free_tree(current_fold_x);
            free_tree(current_fold_y);
        }

        free_tree(current_expr);
        free_tree(current_val);
        next_row(&table);
        start_val += step_val;
    }

    if (g_interactive)
    {
        make_boxed(&table, BORDER_SINGLE);
    }

    set_default_alignments(&table, 3, (TextAlignment[]){ ALIGN_RIGHT, ALIGN_NUMBERS, ALIGN_NUMBERS });
    print_table(&table);
    free_table(&table);

    if (num_args == 6)
    {
        printf("Fold result: " CONSTANT_TYPE_FMT "\n", fold_val);
        core_update_history(fold_val);
    }

    success = true;

    exit:
    free_tree(expr);
    free_tree(start);
    free_tree(end);
    free_tree(step);
    free_tree(fold_expr);
    free_tree(fold_init);

    return success;
}
