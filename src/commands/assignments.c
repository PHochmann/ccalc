#include <stdio.h>
#include <string.h>

#include "assignments.h"
#include "util.h"

#include "../engine/constants.h"
#include "../engine/node.h"
#include "../engine/string_util.h"
#include "../engine/operator.h"
#include "../engine/tokenizer.h"
#include "../engine/parser.h"

#define MSG_ERROR_LEFT "Error in left expression: "
#define MSG_ERROR_RIGHT "Error in right expression: "

void definition_init()
{

}

bool definition_check(char *input)
{
    return strstr(input, ":=") != NULL;
}

void definition_exec(ParsingContext *ctx, char *input)
{
    if (ctx->num_ops == ctx->max_ops)
    {
        printf("Can't add any more functions\n");
        return;
    }

    if (g_num_rules == NUM_MAX_RULES)
    {
        printf("Can't add any more rules\n");
        return;
    }
    
    // Overwrite first char of operator to make function definition a proper string
    char *op_pos = strstr(input, ":=");
    *op_pos = '\0';
    
    // Tokenize function definition to get its name. Name is first token.
    ParserError perr;
    char *tokens[MAX_TOKENS];
    size_t num_tokens = 0;

    if (!tokenize(ctx, input, false, &num_tokens, tokens))
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
    
    if ((perr = parse_input(ctx, input, false, &left_n)) != PERR_SUCCESS)
    {
        ctx->num_ops--;
        free(name);
        printf(MSG_ERROR_LEFT "%s\n", perr_to_string(perr));
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

    if (tree_list_vars(left_n, NULL) != left_n->num_children)
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

    if ((perr = parse_input(ctx, op_pos + 2, false, &right_n)) != PERR_SUCCESS)
    {
        free_tree(left_n);
        free(name);
        printf(MSG_ERROR_RIGHT "%s\n", perr_to_string(perr));
        return;
    }
    
    // Assign correct arity
    ctx->operators[ctx->num_ops].arity = left_n->num_children;
    // Activate added function
    ctx->num_ops++;
    // Add rule to eliminate function before evaluation
    g_rules[g_num_rules++] = get_rule(ctx, left_n, right_n);
    
    whisper("Added function\n");
}


// Rule definition command


void rule_init()
{
    g_num_rules = 0;
}

bool rule_check(char *input)
{
    return strstr(input, "->") != NULL;
}

void rule_exec(ParsingContext *ctx, char *input)
{
    char *op_pos = strstr(input, "->");

    if (g_num_rules == NUM_MAX_RULES)
    {
        printf("Can't add any more rules\n");
        return;
    }
    
    // Overwrite first char of operator to make left hand side a proper string
    *op_pos = '\0';
    
    Node *before_n;
    Node *after_n;
    ParserError perr;
    
    if ((perr = parse_input(ctx, input, false, &before_n)) != PERR_SUCCESS)
    {
        printf("Error in left expression: %s\n", perr_to_string(perr));
        return;
    }
    
    if ((perr = parse_input(ctx, op_pos + 2, false, &after_n)) != PERR_SUCCESS)
    {
        printf("Error in right expression: %s\n", perr_to_string(perr));
        return;
    }
    
    g_rules[g_num_rules++] = get_rule(ctx, before_n, after_n);
    whisper("Rule added\n");
}
