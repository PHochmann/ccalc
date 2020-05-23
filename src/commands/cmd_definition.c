#include <stdio.h>
#include <string.h>

#include "cmd_definition.h"
#include "../string_util.h"
#include "../console_util.h"
#include "../tree/node.h"
#include "../tree/tokenizer.h"
#include "../transformation/rewrite_rule.h"
#include "../core/arith_context.h"

#define DEFINITION_OP   "="
#define FMT_ERROR_LEFT  "Error in left expression: %s\n"
#define FMT_ERROR_RIGHT "Error in right expression: %s\n"

static const size_t MAX_TOKENS = 50;

bool cmd_definition_check(char *input)
{
    return strstr(input, DEFINITION_OP) != NULL;
}

bool do_left_checks(Node *left_n)
{
    if (get_type(left_n) != NTYPE_OPERATOR || get_op(left_n)->placement != OP_PLACE_FUNCTION)
    {
        printf(FMT_ERROR_LEFT, "Not a function or constant.");
        return false;
    }

    for (size_t i = 0; i < get_num_children(left_n); i++)
    {
        if (get_type(get_child(left_n, i)) != NTYPE_VARIABLE)
        {
            printf(FMT_ERROR_LEFT, "Function arguments must be variables.");
            return false;
        }
    }

    if (get_num_children(left_n) != count_variables_distinct(left_n))
    {
        printf(FMT_ERROR_LEFT, "Function arguments must be distinct variables.");
        return false;
    }

    return true;
}

bool add_function(char *name, char *left, char *right)
{
    // Check if left side can be parsed without adding tentative operator
    // If it succeeds, we are redefining an already existing function
    Node *left_n = NULL;
    Node *right_n = NULL;
    RewriteRule *redefined_rule = NULL;

    if (parse_input(g_ctx, left, &left_n) == PERR_SUCCESS)
    {
        if (get_type(left_n) == NTYPE_OPERATOR && get_op(left_n)->placement == OP_PLACE_FUNCTION)
        {
            // Name is not needed since no new function will be added
            free(name);

            // Check if found function is a composite function...
            for (size_t i = 0; i < get_num_composite_functions(); i++)
            {
                if (get_op(get_composite_function(i)->before) == get_op(left_n))
                {
                    redefined_rule = get_composite_function(i);
                }
            }

            // ...if not, the function is built-in. Fail here.
            if (redefined_rule == NULL)
            {
                printf("Error: Built-in functions can not be redefined.\n");
                free_tree(left_n);
                return false;
            }
        }
        else
        {
            free_tree(left_n);
        }
    }
    
    if (redefined_rule == NULL)
    {
        if (!can_add_composite_function())
        {
            printf("Error: Can not add any more functions or constants.\n");
            free(name);
            return false;
        }

        // Add function operator to parse left input
        // Must be OP_DYNAMIC_ARITY because we do not know the actual arity yet
        ctx_add_op(g_ctx, op_get_function(name, OP_DYNAMIC_ARITY));
        if (!core_parse_input(left, FMT_ERROR_LEFT, false, &left_n))
        {
            goto error;
        }
    }

    if (!do_left_checks(left_n)) goto error;

    // Assign correct arity
    get_op(left_n)->arity = get_num_children(left_n);

    if (!core_parse_input(right, FMT_ERROR_RIGHT, false, &right_n))
    {
        goto error;
    }

    if (find_matching_discarded(right_n, left_n))
    {
        printf("Error: Recursive definition.\n");
        goto error;
    }

    if (redefined_rule != NULL)
    {
        printf("Function or constant already exists. Redefine it [y/N]? ");
        
        for (size_t i = 0; i < get_num_composite_functions(); i++)
        {
            RewriteRule *rule = get_composite_function(i);
            if (find_matching_discarded(rule->after, left_n))
            {
                printf("\nWarning: This will affect at least one other function or constant. ");
                break;
            }
        }

        if (!ask_yes_no(false))
        {
            printf("Aborted.\n");
            goto error;
        }
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
        for (size_t i = 0; i < get_num_composite_functions(); i++)
        {
            RewriteRule *rule = get_composite_function(i);
            // Check if variables are unbounded...
            if (count_variable_nodes(rule->before, name) == 0)
            {
                // ...if they are, replace them by new definition
                if (replace_variable_nodes(&rule->after, left_n, name) > 0)
                {
                    replaced_variable = true;
                }
            }
        }

        if (replaced_variable)
        {
            whisper("Note: Unbounded variables in previously defined functions or constants are now bounded.\n");
        }

        if (redefined_rule == NULL)
        {
            whisper("Added constant.\n");
        }
        else
        {
            whisper("Redefined constant.\n");
        }
    }
    else
    {
        if (redefined_rule == NULL)
        {
            whisper("Added function.\n");
        }
        else
        {
            whisper("Redefined function.\n");
        }
    }

    // Add rule to eliminate operator before evaluation
    RewriteRule rule = get_rule(left_n, right_n);
    if (redefined_rule == NULL)
    {
        add_composite_function(rule);
    }
    else
    {
        free_tree(redefined_rule->before);
        free_tree(redefined_rule->after);
        redefined_rule->before = left_n;
        redefined_rule->after = right_n;
    }
    return true;

    error:
    free_tree(left_n);
    free_tree(right_n);
    if (redefined_rule == NULL)
    {
        g_ctx->num_ops--;
        free(name);
    }
    return false;
}

/*
Summary: Adds a new function symbol to context and adds a new rule to substitute function with its right hand side
*/
bool cmd_definition_exec(char *input)
{   
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
        return false;
    }
    
    if (num_tokens > 0)
    {
        // Function name is first token
        char *name = tokens[0];
        // All other tokens can be freed
        for (size_t i = 1; i < num_tokens; i++) free(tokens[i]);

        if (!is_letter(name[0]))
        {
            free(name);
            printf(FMT_ERROR_LEFT, "Functions and constants must only consist of letters.");
            return false;
        }

        return add_function(name, input, right_input);
    }
    else
    {
        // Zero tokens: expression is empty
        printf(FMT_ERROR_LEFT, perr_to_string(PERR_EMPTY));
        return false;
    }
}
