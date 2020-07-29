#include <stdio.h>
#include <string.h>

#include "cmd_definition.h"
#include "../util/string_util.h"
#include "../util/console_util.h"
#include "../tree/node.h"
#include "../tree/tree_util.h"
#include "../parsing/tokenizer.h"
#include "../transformation/rewrite_rule.h"
#include "../core/arith_context.h"

#define DEFINITION_OP   "="
#define FMT_ERROR_LEFT  "Error in left expression: %s\n"
#define FMT_ERROR_RIGHT "Error in right expression: %s\n"

int cmd_definition_check(char *input)
{
    return strstr(input, DEFINITION_OP) != NULL;
}

bool do_left_checks(Node *left_n)
{
    if (get_type(left_n) != NTYPE_OPERATOR || get_op(left_n)->placement != OP_PLACE_FUNCTION)
    {
        report_error(FMT_ERROR_LEFT, "Not a function or constant.");
        return false;
    }

    for (size_t i = 0; i < get_num_children(left_n); i++)
    {
        if (get_type(get_child(left_n, i)) != NTYPE_VARIABLE)
        {
            report_error(FMT_ERROR_LEFT, "Function arguments must be variables.");
            return false;
        }
    }

    if (get_num_children(left_n) != count_variables_distinct(left_n))
    {
        report_error(FMT_ERROR_LEFT, "Function arguments must be distinct variables.");
        return false;
    }

    return true;
}

bool add_function(char *name, char *left, char *right)
{
    // First check if function already exists
    Operator *op = ctx_lookup_op(g_ctx, name, OP_PLACE_FUNCTION);
    if (op != NULL)
    {
        if (op->id < NUM_PREDEFINED_OPS)
        {
            report_error("Built-in functions can not be redefined.\n");
        }
        else
        {
            report_error("Function or constant already defined. Please use clear command before re-definition.\n");
        }
        
        // Don't goto error since no new operator has been added to context
        free(name);
        return false;
    }

    Node *left_n = NULL;
    Node *right_n = NULL;

    // Add function operator to parse left input
    // Must be OP_DYNAMIC_ARITY because we do not know the actual arity yet
    ctx_add_op(g_ctx, op_get_function(name, OP_DYNAMIC_ARITY));
    if (!arith_parse_input_raw(left, FMT_ERROR_LEFT, &left_n))
    {
        goto error;
    }

    // Check if left side is "function(var_1, ..., var_n)"
    if (!do_left_checks(left_n)) goto error;

    // Assign correct arity
    get_op(left_n)->arity = get_num_children(left_n);

    if (!arith_parse_input_raw(right, FMT_ERROR_RIGHT, &right_n))
    {
        goto error;
    }

    if (find_matching_discarded(right_n, left_n, NULL) != NULL)
    {
        report_error("Error: Recursive definition.\n");
        goto error;
    }

    if (get_op(left_n)->arity == 0)
    {
        /*
         * User-defined constants are zero-arity functions with corresponding elimination rule.
         * Previously defined rules do not refer to them, because the string was parsed to a
         * variable node, not an operator. For users, this is confusing, because the technical
         * difference between variables and constant operators is not clear.
         * => Replace unbounded variables of the same name with this new constant.
         */
        bool replaced_variable = false;
        ListNode *curr = g_composite_functions.first;
        while (curr != NULL)
        {
            RewriteRule *rule = (RewriteRule*)curr->data;
            // Check if variables are unbounded...
            if (count_variable_nodes(rule->before, name) == 0)
            {
                // ...if they are, replace them by new definition
                if (replace_variable_nodes(&rule->after, left_n, name) > 0)
                {
                    replaced_variable = true;
                }
            }
            
            curr = curr->next;
        }

        if (replaced_variable)
        {
            whisper("Note: Unbounded variables in previously defined functions or constants are now bounded.\n");
        }

        whisper("Added constant.\n");
    }
    else
    {
        whisper("Added function.\n");
    }

    // Add rule to eliminate operator before evaluation
    RewriteRule rule = get_rule(left_n, right_n, NULL);
    add_composite_function(rule);
    return true;

    error:
    free_tree(left_n);
    free_tree(right_n);
    ctx_delete_op(g_ctx, name, OP_PLACE_FUNCTION);
    free(name);
    return false;
}

/*
Summary: Adds a new function symbol to context and adds a new rule to substitute function with its right hand side
*/
bool cmd_definition_exec(char *input, __attribute__((unused)) int code)
{   
    // Overwrite first char of operator to make function definition a proper string
    char *right_input = strstr(input, DEFINITION_OP);
    *right_input = '\0';
    right_input += strlen(DEFINITION_OP);
    
    // Tokenize function definition to get its name. Name is first token.
    Vector tokens;
    tokenize(input, &g_ctx->keywords_trie, &tokens);
    
    if (vec_count(&tokens) > 0)
    {
        // Function name is first token
        char *name = VEC_GET_ELEM(&tokens, char*, 0);
        // All other tokens can be freed
        for (size_t i = 1; i < vec_count(&tokens); i++)
        {
            free(VEC_GET_ELEM(&tokens, char*, i));
        }
        vec_destroy(&tokens);

        if (!is_letter(name[0]))
        {
            free(name);
            report_error(FMT_ERROR_LEFT, "Not a function or constant.");
            return false;
        }

        return add_function(name, input, right_input);
    }
    else
    {
        // Zero tokens: expression is empty
        report_error(FMT_ERROR_LEFT, perr_to_string(PERR_EMPTY));
        vec_destroy(&tokens);
        return false;
    }
}
