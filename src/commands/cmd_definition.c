#include <stdio.h>
#include <string.h>

#include "../parsing/node.h"
#include "../parsing/tokenizer.h"
#include "../parsing/parser.h"
#include "../matching/matching.h"
#include "../matching/rewrite_rule.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"
#include "cmd_definition.h"
#include "cmd_evaluation.h"
#include "../console_util.h"

#define DEFINITION_OP   ":="
#define FMT_ERROR_LEFT  "Error in left expression: %s.\n"
#define FMT_ERROR_RIGHT "Error in right expression: %s.\n"

static const size_t MAX_TOKENS = 100;

bool cmd_definition_check(char *input)
{
    return strstr(input, DEFINITION_OP) != NULL;
}

void add_function(char *name, char *left, char *right)
{
    // Add function operator to parse left input
    // Must be DYNAMIC_ARITY because we do not know the actual arity yet
    ctx_add_op(g_ctx, op_get_function(name, DYNAMIC_ARITY));

    Node *left_n = NULL;
    Node *right_n = NULL;
    size_t num_distinct_vars;
    
    if (!parse_input_from_console(g_ctx, left, FMT_ERROR_LEFT, &left_n, false, false))
    {
        goto cleanup;
    }

    if (left_n->type != NTYPE_OPERATOR || left_n->op->placement != OP_PLACE_FUNCTION)
    {
        printf(FMT_ERROR_LEFT, "Not a function");
        goto cleanup;
    }
    
    for (size_t i = 0; i < left_n->num_children; i++)
    {
        if (left_n->children[i]->type != NTYPE_VARIABLE)
        {
            printf(FMT_ERROR_LEFT, "Function arguments must be variables");
            goto cleanup;
        }
    }

    if (!tree_count_vars_distinct(left_n, &num_distinct_vars))
    {
        printf(FMT_ERROR_LEFT, "Max. arity of function exceeded");
        goto cleanup;
    }

    if (num_distinct_vars != left_n->num_children)
    {
        printf(FMT_ERROR_LEFT, "Function arguments must be distinct variables");
        goto cleanup;
    }

    if (ctx_lookup_function(g_ctx, name, left_n->num_children) != NULL)
    {
        printf("Function already exists\n");
        goto cleanup;
    }

    // Assign correct arity
    g_ctx->operators[g_ctx->num_ops - 1].arity = left_n->num_children;

    if (!parse_input_from_console(g_ctx, right, FMT_ERROR_RIGHT, &right_n, false, false))
    {
        goto cleanup;
    }

    if (find_matching_discarded(g_ctx, right_n, left_n))
    {
        printf("Recursive function definition\n");
        goto cleanup;
    }

    whisper("Added function.\n");

    // Add rule to eliminate operator before evaluation
    g_rules[g_num_rules++] = get_rule(left_n, right_n);
    return;

    cleanup:
    g_ctx->num_ops--;
    free_tree(left_n);
    free_tree(right_n);
    free(name);
}

void add_constant(char *name, char *right)
{
    Node *right_n = NULL;
    size_t var_count = 0;

    if (ctx_lookup_op(g_ctx, name, OP_PLACE_PREFIX))
    {
        printf("Constant already exists\n");
        goto cleanup;
    }

    if (!parse_input_from_console(g_ctx, right, FMT_ERROR_RIGHT, &right_n, false, false))
    {
        goto cleanup;
    }

    if (!tree_count_var_instances(right_n, name, &var_count))
    {
        printf(FMT_ERROR_RIGHT, "Max. stack size exceeded or too many variables used\n");
        goto cleanup;
    }

    if (var_count > 0)
    {
        printf(FMT_ERROR_RIGHT, "Recursive constant definition\n");
        goto cleanup;
    }

    whisper("Added constant.\n");

    // Add new constant to parsing context and get a node containing it
    ctx_add_op(g_ctx, op_get_constant(name));
    Node *op_node = malloc_node(
        get_operator_node(&g_ctx->operators[g_ctx->num_ops - 1], 0));

    /*
    User-defined constants are parser-level operators with corresponding elimination rule.
    Previously defined constants and functions do not refer to them, because the string was parsed
    to a variable node, not an operator. For users, this is confusing, because the difference
    between variables and constant operators is not clear. Thus, replace variables of the same name
    with this new constant.
    */
    for (size_t i = ARITH_NUM_RULES; i < g_num_rules; i++)
    {
        tree_substitute_var(g_ctx, g_rules[i].after, op_node, name);
    }

    // Add elimination rule for constant
    g_rules[g_num_rules++] = get_rule(op_node, right_n);
    return;

    cleanup:
    free(name);
}

/*
Summary: Adds a new function symbol to context and adds a new rule to substitute function with its right hand side
*/
void cmd_definition_exec(char *input)
{
    if (g_ctx->num_ops == g_ctx->max_ops || g_num_rules == ARITH_MAX_RULES)
    {
        printf("Can't add any more functions or constants\n");
        return;
    }
    
    // Overwrite first char of operator to make function definition a proper string
    char *right_input = strstr(input, DEFINITION_OP);
    *right_input = '\0';
    right_input += strlen(DEFINITION_OP);
    
    // Tokenize function definition to get its name. Name is first token.
    char *tokens[MAX_TOKENS];
    size_t num_tokens = 0;

    if (!tokenize(g_ctx, input, MAX_TOKENS, &num_tokens, tokens))
    {
        // Only reason for tokenize to fail is max. number of tokens exceeded
        printf(FMT_ERROR_LEFT, perr_to_string(PERR_MAX_TOKENS_EXCEEDED));
        return;
    }
    
    char *name = NULL;
    
    if (num_tokens > 0)
    {
        name = tokens[0];
        // Free tokens and pointers to them
        for (size_t i = 1; i < num_tokens; i++) free(tokens[i]);

        if (!is_letter(name[0]))
        {
            free(name);
            printf(FMT_ERROR_LEFT, "Functions and constants must only consist of letters");
            return;
        }

        if (num_tokens == 1)
        {
            add_constant(name, right_input);
        }
        else
        {
            add_function(name, input, right_input);
        }
    }
    else
    {
        // Zero tokens: expression is empty
        printf(FMT_ERROR_LEFT, perr_to_string(PERR_EMPTY));
        return;
    }
}
