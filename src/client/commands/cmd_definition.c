#include <stdio.h>
#include <string.h>

#include "../../util/string_util.h"
#include "../../util/console_util.h"
#include "../../engine/tree/node.h"
#include "../../engine/tree/tree_util.h"
#include "../../engine/parsing/tokenizer.h"
#include "../../engine/parsing/parser.h"
#include "../../engine/transformation/rewrite_rule.h"

#include "cmd_definition.h"
#include "../core/arith_context.h"

#define DEFINITION_OP   "="

#define ERR_NOT_A_FUNC                "Error: Not a function or constant"
#define ERR_ARGS_NOT_VARS             "Error: Function arguments must be variables"
#define ERR_NOT_DISTINCT              "Error: Function arguments must be distinct variables"
#define ERR_NEW_VARIABLE_INTRODUCTION "Error: Unbound variable\n"
#define ERR_BUILTIN_REDEFINITION      "Error: Built-in functions can not be redefined\n"
#define ERR_REDEFINITION              "Error: Function or constant already defined. Use clear command before redefinition\n"
#define ERR_RECURSIVE_DEFINITION      "Error: Recursive definition\n"

int cmd_definition_check(const char *input)
{
    return strstr(input, DEFINITION_OP) != NULL;
}

static bool do_left_checks(Node *left_n, int strlen)
{
    if (get_type(left_n) != NTYPE_OPERATOR || get_op(left_n)->placement != OP_PLACE_FUNCTION)
    {
        show_error_with_position(0, strlen, ERR_NOT_A_FUNC);
        return false;
    }

    size_t num_children = get_num_children(left_n);

    if (num_children > 0)
    {
        for (size_t i = 0; i < num_children; i++)
        {
            if (get_type(get_child(left_n, i)) != NTYPE_VARIABLE)
            {
                show_error_with_position(0, strlen, ERR_ARGS_NOT_VARS);
                return false;
            }
        }

        const char *vars[MAX_MAPPED_VARS];
        bool sufficient_buff = false;
        size_t num_vars = list_variables(left_n, MAX_MAPPED_VARS, vars, &sufficient_buff);
        if (!sufficient_buff)
        {
            show_error_with_position(0, strlen, "Too many function parameters. Maximum is %zu.", MAX_MAPPED_VARS);
            return false;
        }
        if (num_vars != num_children)
        {
            show_error_with_position(0, strlen, ERR_NOT_DISTINCT);
            return false;
        }
    }

    return true;
}

static bool add_function(char *name, char *left, char *right, bool is_constant)
{
    // First check if function already exists
    const Operator *op = ctx_lookup_op(g_ctx, name, OP_PLACE_FUNCTION);
    if (op != NULL)
    {
        if (op->id < NUM_ARITH_OPS)
        {
            report_error(ERR_BUILTIN_REDEFINITION);
        }
        else
        {
            report_error(ERR_REDEFINITION);
        }
        
        // Don't goto error since no new operator has been added to context
        free(name);
        return false;
    }

    // Add function operator to parse left input
    // Must be OP_DYNAMIC_ARITY because we do not know the actual arity yet
    ParsingResult left_result = { .error = PERR_NULL };
    ParsingResult right_result = { .error = PERR_NULL };
    const Operator *new_op = NULL;

    // To successfully parse inputs like "x = 5", we can't add a function with dynamic arity because
    // the user would have to type "x() = 5" since dynamic arity functions require parameter lists
    if (!is_constant)
    {
        ctx_add_op(g_ctx, op_get_function(name, OP_DYNAMIC_ARITY));
    }
    else
    {
        new_op = ctx_add_op(g_ctx, op_get_constant(name));
    }

    if (!arith_parse_raw(left, 0, &left_result))
    {
        goto error;
    }

    // Check if left side is "function(var_1, ..., var_n)"
    if (!do_left_checks(left_result.tree, strlen(left)))
    {
        goto error;
    }

    // Assign correct arity:
    // Since operators are const, we can't change the arity directly
    // A new operator with correct arity has to be created
    if (!is_constant)
    {
        ctx_delete_op(g_ctx, name, OP_PLACE_FUNCTION);
        new_op = ctx_add_op(g_ctx, op_get_function(name, get_num_children(left_result.tree)));
        set_op(left_result.tree, new_op);
    }

    // Parse right expression raw to detect a recursive definition
    if (!arith_parse_raw(right, (size_t)(right - left), &right_result))
    {
        goto error;
    }

    // Check if function is used in its definition
    if (find_op((const Node**)&right_result.tree, new_op) != NULL)
    {
        report_error(ERR_RECURSIVE_DEFINITION);
        goto error;
    }

    // Since right expression was parsed raw to detect recursive definitions, do postprocessing
    if (!arith_postprocess(&right_result, (size_t)(right - left)))
    {
        goto error;
    }

    // Add rule to eliminate operator before evaluation
    RewriteRule rule;
    Pattern pattern;
    get_pattern(left_result.tree, 0, NULL, &pattern); // Should always succeed
    if (!get_rule(pattern, right_result.tree, &rule)) // Only reason to return false is new variable introduction
    {
        report_error(ERR_NEW_VARIABLE_INTRODUCTION);
        goto error;
    }

    add_composite_function(rule);

    if (get_op(left_result.tree)->arity == 0)
    {
        if (!is_constant)
        {
            whisper("Added constant. Note: constants don't require a parameter list.\n");
        }
        else
        {
            whisper("Added constant.\n");
        }
    }
    else
    {
        whisper("Added function.\n");
    }
    free_result(&left_result, false);
    free_result(&right_result, false);
    return true;

    error:
    free_result(&left_result, true);
    free_result(&right_result, true);
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
    
    // Function name is first token that is not a space
    char *name = NULL;
    size_t non_space_tokens = 0;
    for (size_t i = 0; i < vec_count(&tokens); i++)
    {
        char *token = *(char**)vec_get(&tokens, i);
        if (is_space(token[0]))
        {
            free(token);
        }
        else
        {
            non_space_tokens++;
            if (name == NULL)
            {
                name = token;
            }
            else
            {
                free(token);
            }
        }
    }

    vec_destroy(&tokens);

    if (name == NULL)
    {
        show_error_with_position(0, strlen(input), ERR_NOT_A_FUNC);
        return false;
    }
    else
    {
        if (!is_letter(name[0]))
        {
            free(name);
            show_error_with_position(0, strlen(input), ERR_NOT_A_FUNC);
            return false;
        }
        else
        {
            return add_function(name, input, right_input, non_space_tokens == 1);
        }
    }
}
