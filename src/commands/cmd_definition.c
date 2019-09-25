#include <stdio.h>
#include <string.h>

#include "../parsing/node.h"
#include "../parsing/tokenizer.h"
#include "../parsing/parser.h"
#include "../matching/rewrite_rule.h"
#include "../arithmetics/arith_rules.h"
#include "cmd_definition.h"
#include "cmd_evaluation.h"
#include "console_util.h"

#define DEFINITION_OP   ":="
#define MSG_ERROR_LEFT  "Error in left expression: "
#define MSG_ERROR_RIGHT "Error in right expression: "

static const size_t MAX_TOKENS = 100;

void cmd_definition_init() { }

bool cmd_definition_check(char *input)
{
    return strstr(input, DEFINITION_OP) != NULL;
}

/*
Summary: Adds a new function symbol to context and adds a new rule to substitute function with its right hand side
*/
void cmd_definition_exec(ParsingContext *ctx, char *input)
{
    if (ctx->num_ops == ctx->max_ops)
    {
        printf("Can't add any more functions\n");
        return;
    }

    if (g_num_rules == ARITH_MAX_RULES)
    {
        printf("Can't add any more rules\n");
        return;
    }
    
    // Overwrite first char of operator to make function definition a proper string
    char *right_input = strstr(input, DEFINITION_OP);
    *right_input = '\0';
    right_input += strlen(DEFINITION_OP);
    
    // Tokenize function definition to get its name. Name is first token.
    char *tokens[MAX_TOKENS];
    size_t num_tokens = 0;

    if (!tokenize(ctx, input, MAX_TOKENS, &num_tokens, tokens))
    {
        // Only reason for tokenize to fail is max. number of tokens exceeded
        printf(MSG_ERROR_LEFT "%s\n", perr_to_string(PERR_MAX_TOKENS_EXCEEDED));
        return;
    }
    
    char *name = NULL;
    
    if (num_tokens > 0)
    {
        name = tokens[0];
        // Free tokens and pointers to them
        for (size_t i = 1; i < num_tokens; i++) free(tokens[i]);
        ctx_add_op(ctx, op_get_function(name, DYNAMIC_ARITY));
    }
    else
    {
        // Zero tokens: expression is empty
        printf(MSG_ERROR_LEFT "%s\n", perr_to_string(PERR_EMPTY));
        return;
    }
    
    Node *left_n;
    
    if (!parse_input_console(ctx, input, MSG_ERROR_LEFT "%s\n", &left_n, false, false))
    {
        ctx->num_ops--;
        free(name);
        return;
    }

    // Disable newly added function while parsing right-hand side to avoid recursive function
    ctx->num_ops--;
    
    if (left_n->type != NTYPE_OPERATOR || left_n->op->placement != OP_PLACE_FUNCTION)
    {
        free_tree(left_n);
        free(name);
        printf(MSG_ERROR_LEFT "Not a function\n");
        return;
    }
    
    for (size_t i = 0; i < left_n->num_children; i++)
    {
        if (left_n->children[i]->type != NTYPE_VARIABLE)
        {
            free_tree(left_n);
            free(name);
            printf(MSG_ERROR_LEFT "Arguments must be variables\n");
            return;
        }
    }

    size_t num_distinct_vars;
    if (!tree_list_vars(left_n, &num_distinct_vars, NULL))
    {
        free_tree(left_n);
        free(name);
        printf(MSG_ERROR_LEFT "Max. arity of function exceeded\n");
        return;
    }

    if (num_distinct_vars != left_n->num_children)
    {
        free_tree(left_n);
        free(name);
        printf(MSG_ERROR_LEFT "Arguments must be distinct variables\n");
        return;
    }

    if (ctx_lookup_function(ctx, name, left_n->num_children) != NULL)
    {
        free_tree(left_n);
        free(name);
        printf("Function already exists\n");
        return;
    }
    
    Node *right_n;

    if (!parse_input_console(ctx, right_input, MSG_ERROR_RIGHT "%s\n", &right_n, false, false))
    {
        free_tree(left_n);
        free(name);
        return;
    }

    // Assign correct arity
    ctx->operators[ctx->num_ops].arity = left_n->num_children;
    // Activate added function
    ctx->num_ops++;
    // Add rule to eliminate function before evaluation
    g_rules[g_num_rules++] = get_rule(left_n, right_n);
    
    whisper("Added function\n");
}
