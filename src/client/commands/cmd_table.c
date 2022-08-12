#include <string.h>

#include "../../util/console_util.h"
#include "../../util/string_util.h"
#include "../../util/string_builder.h"
#include "../../engine/tree/tree_to_string.h"
#include "../../engine/tree/tree_util.h"
#include "../../table/table.h"
#include "../core/arith_context.h"
#include "../core/history.h"
#include "../core/arith_evaluation.h"
#include "cmd_table.h"

#define COMMAND      "table "
#define FOLD_KEYWORD " fold "
#define FOLD_VAR_1   "x"
#define FOLD_VAR_2   "y"

#define STRBUILDER_STARTSIZE 10
#define DOUBLE_FMT "%f"

int cmd_table_check(const char *input)
{
    return begins_with(COMMAND, input);
}

bool check_if_constant(const char *offset, const char *string, const Node *node)
{
    if (count_all_variable_nodes(node) > 0)
    {
        report_error_at(string - offset, strlen(string), "Error: Not constant");
        return false;
    }
    return true;
}

bool cmd_table_exec(char *input, __attribute__((unused)) int code)
{
    char *args[6];
    size_t num_args = str_split(input + strlen(COMMAND), args, 5, ";", ";", ";", FOLD_KEYWORD, ";");

    if (num_args != 4 && num_args != 6)
    {
        report_error("Error: Invalid syntax. Syntax is:\n"
               "table <expr> ; <from> ; <to> ; <step> [fold <expr> ; <init>]\n");
        return false;
    }

    for (size_t i = 0; i < num_args; i++)
    {
        args[i] = strip(args[i]);
    }

    bool success = false;
    Node *expr = NULL;
    Node *start = NULL;
    Node *end = NULL;
    Node *step = NULL;
    Node *fold_expr = NULL;
    Node *fold_init = NULL;
    char *expr_string = NULL;

    ParsingResult presult = { .error = PERR_NULL };
    if (!arith_parse_raw(args[0], (size_t)(args[0] - input), &presult))
    {
        free_result(&presult, false);
        return false;
    }

    Vector builder = strbuilder_create(STRBUILDER_STARTSIZE);
    strbuilder_append(&builder, " ");
    tree_append_to_strbuilder(&builder, presult.tree, g_ctx, true);
    strbuilder_append(&builder, " ");
    expr_string = strbuilder_to_str(&builder);

    expr = arith_simplify(&presult, args[0] - input);
    if (expr == NULL)
    {
        goto exit;
    }

    const char *var;
    bool sufficient = false;
    size_t num_vars = list_variables(expr, 1, &var, &sufficient);
    if (!sufficient)
    {
        report_error_at(args[0] - input, strlen(args[0]), "Error: More than one variable\n");
        goto exit;
    }

    if (!arith_parse(args[1], (size_t)(args[1] - input), &start)
        || !arith_parse(args[2], (size_t)(args[2] - input), &end)
        || !arith_parse(args[3], (size_t)(args[3] - input), &step))
    {
        goto exit;
    }

    if (!check_if_constant(input, args[1], start)
        || !check_if_constant(input, args[2], end)
        || !check_if_constant(input, args[3], step))
    {
        goto exit;
    }

    double start_val = arith_evaluate(start);
    double end_val = arith_evaluate(end);
    double step_val = arith_evaluate(step);

    if (step_val == 0)
    {
        report_error_at(args[3] - input, strlen(args[3]), "Error: 'step' must not be zero\n");
        goto exit;
    }

    // Optionally: Parse part of command after "fold"
    double fold_val = 0;
    if (num_args == 6)
    {
        // Parse initial fold-value
        if (!arith_parse(args[4], (size_t)(args[4] - input), &fold_expr)
            || !arith_parse(args[5], (size_t)(args[5] - input), &fold_init))
        {
            goto exit;
        }

        if (count_all_variable_nodes(fold_expr)
            - get_variable_nodes((const Node**)&fold_expr, FOLD_VAR_1, 0, NULL)
            - get_variable_nodes((const Node**)&fold_expr, FOLD_VAR_2, 0, NULL) != 0)
        {
            report_error_at(args[4] - input, strlen(args[4]),
                "Error: Fold expression must not contain any variables except '" FOLD_VAR_1 "' and '" FOLD_VAR_2 "'\n");
            goto exit;
        }

        if (!check_if_constant(input, args[5], fold_init)) goto exit;

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
    
    // Print header row only if interactive
    if (is_interactive())
    {
        add_empty_cell(table);
        if (num_vars != 0)
        {
            add_cell_fmt(table, VAR_COLOR " %s " COL_RESET, var);
        }
        else
        {
            // Expression is constant - don't print any variable
            add_empty_cell(table);
        }
        add_cell(table, expr_string);
        next_row(table);
    }

    // Loop through all values and add them to table
    for (size_t i = 1; step_val > 0 ? start_val <= end_val : start_val >= end_val; i++)
    {
        Node *current_expr = tree_copy(expr);
        Node *current_val = malloc_constant_node(start_val, 0);
        replace_variable_nodes(&current_expr, current_val, var);

        double result = 0;
        ListenerError err = tree_reduce(current_expr, arith_op_evaluate, &result, NULL);

        if (is_interactive()) add_cell_fmt(table, " %zu ", i);
        add_cell_fmt(table, " " DOUBLE_FMT " ", start_val);

        if (err == LISTENERERR_SUCCESS)
        {
            add_cell_fmt(table, " " DOUBLE_FMT " ", result);

            if (num_args == 6)
            {
                Node *current_fold = tree_copy(fold_expr);
                Node *current_fold_x = malloc_constant_node(fold_val, 0);
                Node *current_fold_y = malloc_constant_node(result, 0);
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
            add_cell_fmt(table, " Error ");
        }

        free_tree(current_expr);
        free_tree(current_val);
        next_row(table);
        start_val += step_val;
    }

    set_default_alignments(table, 3, (TableHAlign[]){ H_ALIGN_RIGHT, H_ALIGN_RIGHT, H_ALIGN_RIGHT }, NULL);
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
    free(expr_string);
    free_tree(start);
    free_tree(end);
    free_tree(step);
    free_tree(fold_expr);
    free_tree(fold_init);
    return success;
}
