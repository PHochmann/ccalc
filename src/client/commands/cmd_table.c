#include <string.h>

#include "../../engine/util/console_util.h"
#include "../../engine/util/string_util.h"
#include "../../engine/util/string_builder.h"
#include "../../engine/tree/tree_to_string.h"
#include "../../engine/tree/tree_util.h"
#include "../../engine/table/table.h"
#include "../core/arith_context.h"
#include "../core/history.h"
#include "../core/arith_evaluation.h"
#include "cmd_table.h"

#define COMMAND      "table "
#define FOLD_KEYWORD " fold "
#define FOLD_VAR_1   "x"
#define FOLD_VAR_2   "y"

#define STRBUILDER_STARTSIZE 10

int cmd_table_check(const char *input)
{
    return begins_with(COMMAND, input);
}

bool cmd_table_exec(char *input, __attribute__((unused)) int code)
{
    input += strlen(COMMAND);
    char *args[6];
    size_t num_args = str_split(input, args, 5, ";", ";", ";", FOLD_KEYWORD, ";");

    if (num_args != 4 && num_args != 6)
    {
        report_error("Error: Invalid syntax. Syntax is:\n"
               "table <expr> ; <from> ; <to> ; <step> [fold <expr> ; <init>]\n");
        return false;
    }

    bool success = false;
    Node *expr = NULL;
    Node *start = NULL;
    Node *end = NULL;
    Node *step = NULL;
    Node *fold_expr = NULL;
    Node *fold_init = NULL;

    if (!arith_parse_and_postprocess(args[0], "Error in expression: %s\n", &expr))
    {
        return false;
    }

    const char *var;
    bool sufficient = false;
    size_t num_vars = list_variables(expr, 1, &var, &sufficient);
    if (!sufficient)
    {
        report_error("Error: Expression contains more than one variable\n");
        goto exit;
    }

    if (!arith_parse_and_postprocess(args[1], "Error in start: %s\n", &start)
        || !arith_parse_and_postprocess(args[2], "Error in end: %s\n", &end)
        || !arith_parse_and_postprocess(args[3], "Error in step: %s\n", &step))
    {
        goto exit;
    }

    if (count_all_variable_nodes(start) > 0
        || count_all_variable_nodes(end) > 0
        || count_all_variable_nodes(step) > 0)
    {
        report_error("Error: Start, end and step must be constant\n");
        goto exit;
    }

    double start_val = arith_evaluate(start);
    double end_val = arith_evaluate(end);
    double step_val = arith_evaluate(step);

    if (step_val == 0)
    {
        report_error("Error: Step must not be zero\n");
        goto exit;
    }

    // Optionally: Parse part of command after "fold"
    double fold_val = 0;
    if (num_args == 6)
    {
        // Parse initial fold-value
        if (!arith_parse_and_postprocess(args[4], "Error in fold expression: %s\n", &fold_expr)
            || !arith_parse_and_postprocess(args[5], "Error in initial fold value: %s\n", &fold_init))
        {
            goto exit;
        }

        if (count_all_variable_nodes(fold_expr)
            - get_variable_nodes((const Node**)&fold_expr, FOLD_VAR_1, 0, NULL)
            - get_variable_nodes((const Node**)&fold_expr, FOLD_VAR_2, 0, NULL) != 0)
        {
            report_error("Error: Fold expression must not contain any variables except '"
                FOLD_VAR_1 "' and '" FOLD_VAR_2 "'\n");
            goto exit;
        }

        if (count_all_variable_nodes(fold_init) > 0)
        {
            report_error("Error: Initial fold value must be constant\n");
            goto exit;
        }

        fold_val = arith_evaluate(fold_init);
    }
    // - - - End of parsing of fold-construct

    // Adjust step direction to not have an endless loop
    if ((start_val < end_val && step_val < 0)
        || (start_val > end_val && step_val > 0))
    {
        step_val *= -1;
    }

    Table *table = get_empty_table();
    
    add_empty_cell(table);
    if (num_vars != 0)
    {
        add_cell_fmt(table, VAR_COLOR " %s " COL_RESET, var);
    }
    else
    {
        add_empty_cell(table);
    }

    Vector builder = strbuilder_create(STRBUILDER_STARTSIZE);
    strbuilder_append(&builder, " ");
    tree_to_strbuilder(&builder, expr, true);
    strbuilder_append(&builder, " ");
    add_cell_gc(table, builder.buffer);
    override_alignment_of_row(table, ALIGN_LEFT);
    next_row(table);

    // Loop through all values and add them to table
    for (size_t i = 1; step_val > 0 ? start_val <= end_val : start_val >= end_val; i++)
    {
        Node *current_expr = tree_copy(expr);
        Node *current_val = malloc_constant_node(start_val);
        replace_variable_nodes(&current_expr, current_val, var);

        double result = 0;
        ListenerError err = tree_reduce(current_expr, arith_op_evaluate, &result);

        add_cell_fmt(table, " %zu ", i);
        add_cell_fmt(table, " " CONSTANT_TYPE_FMT " ", start_val);

        if (err == LISTENERERR_SUCCESS)
        {
            add_cell_fmt(table, " " CONSTANT_TYPE_FMT " ", result);

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
        }
        else
        {
            add_cell_fmt(table, " %s ", listenererr_to_str(err));
        }

        free_tree(current_expr);
        free_tree(current_val);
        next_row(table);
        start_val += step_val;
    }

    set_default_alignments(table, 3, (TextAlignment[]){ ALIGN_RIGHT, ALIGN_NUMBERS, ALIGN_NUMBERS });
    print_table(table);
    free_table(table);

    if (num_args == 6) // Contains fold expression
    {
        printf("Fold result: " CONSTANT_TYPE_FMT "\n", fold_val);
        history_add(fold_val);
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
