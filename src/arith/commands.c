#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "commands.h"
#include "arith_ctx.h"

#include "../engine/constants.h"
#include "../engine/node.h"
#include "../engine/operator.h"
#include "../engine/tokenizer.h"
#include "../engine/parser.h"
#include "../engine/rule.h"
#include "../engine/memory.h"
#include "../engine/console_util.h"

#define VERSION "1.1.3"
#define MAX_LINE_LENGTH 256
#define NUM_MAX_RULES 16

void parse_evaluation(char *input);
void print_help();
bool ask_input(char *prompt, char **out_input);
bool parse_input_wrapper(char *input, Node **out_res, bool apply_rules, bool apply_ans);
void parse_assignment(char *input, char *op);
void parse_rule(char *input, char *op);
void message(int prio, const char *format, ...);

static bool debug; // When set to true, a tree and string representation is printed
static int min_prio; // Minimal priority of messages that are printed

static ParsingContext *ctx; // ParsingContext to use while parsing strings
static Node *ans; // Last parsed tree 'ans' is substituted with

static RewriteRule rules[NUM_MAX_RULES];
static int num_rules;

/*
Summary: Sets parsing context and prepares global vars
*/
void init_commands()
{
#ifdef DEBUG
    debug = true;
#else
    debug = false;
#endif

    ans = NULL;
    ctx = arith_get_ctx();
    num_rules = 0;
}

/*
Summary: Sets min_prio to 1, thus messages with priority of 0 are filtered
*/
void make_silent()
{
    min_prio = 1;
}

/*
Summary: Endless loop to ask user for input
*/
void main_interactive()
{
    char *input;
    rl_bind_key('\t', rl_insert); // Disable tab completion
    
#ifdef DEBUG
    printf("Calculator %s Debug build (c) 2018, Philipp Hochmann\n", VERSION);
#else
    printf("Calculator %s (c) 2018, Philipp Hochmann\n", VERSION);
#endif
    printf("Commands: debug, help, <function> := <after>, <before> -> <after>\n");
    
    while (true)
    {
        if (ask_input("> ", &input))
        {
            parse_command(input);
            free(input);
        }
        else return;
    }
}

bool ask_input(char *prompt, char **out_input)
{
    *out_input = readline(prompt);
    if (!(*out_input)) return false;
    add_history(*out_input);
    return true;
}

void parse_command(char *input)
{
    if (strcmp(input, "quit") == 0)
    {
        exit(0);
    }
    
    if (strcmp(input, "help") == 0)
    {
        print_help();
        return;
    }
    
    if (strcmp(input, "debug") == 0)
    {
        debug = !debug;
        message(0, "toggled debug\n");
        return;
    }
    
    char *op_pos = strstr(input, ":=");
    if (op_pos != NULL)
    {
        parse_assignment(input, op_pos);
        return;
    }
    
    op_pos = strstr(input, "->");
    if (op_pos != NULL)
    {
        parse_rule(input, op_pos);
        return;
    }
    
    parse_evaluation(input);
}

void parse_assignment(char *input, char *op_pos)
{
    if (ctx->num_ops == ctx->max_ops)
    {
        printf("Can't add any more operators\n");
        return;
    }
    
    // Overwrite first char of operator to make function definition a proper string
    *op_pos = '\0';
    
    // Tokenize function definition to get its name. Name is first token.
    char **tokens;
    int num_tokens = 0;
    if (!tokenize(ctx, input, &tokens, &num_tokens))
    {
        printf("Error in function definition\n");
    }
    
    char *name = NULL;
    
    if (num_tokens > 0)
    {
        name = tokens[0];
        // Free tokens and pointers to them
        for (int i = 1; i < num_tokens; i++) free(tokens[i]);
        free(tokens);
        ctx_add_op(ctx, op_get_function(name, DYNAMIC_ARITY));
    }
    else
    {
        printf("Error in function definition\n");
        return;
    }
    
    Node *left_n;
    
    if (parse_input(ctx, input, &left_n) != PERR_SUCCESS)
    {
        ctx->num_ops--;
        printf("Error in function definition\n");
        return;
    }
    
    if (left_n->type != NTYPE_OPERATOR || left_n->op->placement != OP_PLACE_FUNCTION)
    {
        ctx->num_ops--;
        free_tree(left_n);
        free(name);
        printf("Error in function definition\n");
        return;
    }
    
    for (size_t i = 0; i < left_n->num_children; i++)
    {
        if (left_n->children[i]->type != NTYPE_VARIABLE)
        {
            ctx->num_ops--;
            free_tree(left_n);
            free(name);
            printf("Error in function definition\n");
            return;
        }
    }
    
    if (ctx_lookup_function(ctx, name, left_n->num_children) != NULL)
    {
        ctx->num_ops--;
        free_tree(left_n);
        free(name);
        printf("Function already exists\n");
        return;
    }
    
    ctx->operators[ctx->num_ops - 1].arity = left_n->num_children;
    
    if (num_rules == NUM_MAX_RULES)
    {
        ctx->num_ops--;
        free_tree(left_n);
        free(name);
        printf("Can't add any more rules\n");
        return;
    }
    
    Node *right_n;
    
    ParserError perr;
    if ((perr = parse_input(ctx, op_pos + 2, &right_n)) != PERR_SUCCESS)
    {
        ctx->num_ops--;
        free_tree(left_n);
        free(name);
        printf("Error in right expression: %s\n", perr_to_string(perr));
        return;
    }
    
    rules[num_rules++] = get_rule(ctx, left_n, right_n);
    message(0, "Added function\n");
}

void parse_rule(char *input, char *op_pos)
{
    if (num_rules == NUM_MAX_RULES)
    {
        printf("Can't add any more rules\n");
        return;
    }
    
    // Overwrite first char of operator to make left hand side a proper string
    *op_pos = '\0';
    
    Node *before_n;
    Node *after_n;
    ParserError perr;
    
    if ((perr = parse_input(ctx, input, &before_n)) != PERR_SUCCESS)
    {
        printf("Error in left expression: %s\n", perr_to_string(perr));
        return;
    }
    
    if ((perr = parse_input(ctx, op_pos + 2, &after_n)) != PERR_SUCCESS)
    {
        printf("Error in right expression: %s\n", perr_to_string(perr));
        return;
    }
    
    rules[num_rules++] = get_rule(ctx, before_n, after_n);
    message(0, "Rule added\n");
}

void parse_evaluation(char *input)
{
    Node *res;
    
    if (parse_input_wrapper(input, &res, true, true))
    {
        if (debug)
        {
            print_tree_visual(ctx, res);
            printf("= ");
            print_tree_inline(ctx, res);
            printf("\n");
        }
        
        char *vars[MAX_VAR_COUNT];
        int num_variables = tree_list_variables(res, vars);
        for (int i = 0; i < num_variables; i++)
        {
            printf(" %s? ", vars[i]);
            char *input;
            if (ask_input("> ", &input))
            {
                Node *res_var;
                if (!parse_input_wrapper(input, &res_var, true, true))
                {
                    // Error while parsing - ask again
                    free(input);
                    i--;
                    continue;
                }
                free(input);
                
                if (tree_contains_variable(res_var))
                {
                    // Not a constant given - ask again
                    printf("Not a constant expression\n");
                    free_tree(res_var);
                    i--;
                    continue;
                }
                
                tree_substitute_variable(ctx, res, res_var, vars[i]);
                free(vars[i]);
                free_tree(res_var);
            }
            else return;
        }
        
        double eval = arith_eval(res);
        char result_str[ctx->min_str_len];
        ctx->to_string((void*)(&eval), result_str, ctx->min_str_len);
        printf("= %s\n", result_str);
        
        if (ans != NULL) free_tree(ans);
        ans = res;
    }
}

bool parse_input_wrapper(char *input, Node **out_res, bool apply_rules, bool apply_ans)
{
    ParserError perr = parse_input(ctx, input, out_res);
    if (perr != PERR_SUCCESS)
    {
        printf("Error: %s\n", perr_to_string(perr));
        return false;
    }
    
    if (apply_ans && ans != NULL) tree_substitute_variable(ctx, *out_res, ans, "ans");
    if (apply_rules)
    {
        apply_ruleset(*out_res, rules, num_rules, 50);
    }
    
    return true;
}

void print_help()
{
    for (int i = 0; i < ctx->num_ops; i++)
    {
        printf(OP_COLOR);
        switch (ctx->operators[i].placement)
        {
            case OP_PLACE_PREFIX:
                if (ctx->operators[i].arity != 0)
                {
                    printf("%sx", ctx->operators[i].name);
                }
                else
                {
                    printf("%s", ctx->operators[i].name);
                }
                break;
                
            case OP_PLACE_INFIX:
                if (strlen(ctx->operators[i].name) == 1)
                {
                    printf("x%sy", ctx->operators[i].name);
                }
                else
                {
                    printf("x %s y", ctx->operators[i].name);
                }
                break;
                
            case OP_PLACE_POSTFIX:
                printf("x%s", ctx->operators[i].name);
                break;
                
            case OP_PLACE_FUNCTION:
                if (ctx->operators[i].arity != DYNAMIC_ARITY)
                {
                    printf("%s(%d)", ctx->operators[i].name, ctx->operators[i].arity);
                }
                else
                {
                    printf("%s(0..%d)", ctx->operators[i].name, MAX_CHILDREN);
                }
                break;
        }
        printf(COL_RESET " ");
    }
    printf("\n(%d available operators)\n", ctx->num_ops);
}

/*
Summary: printf-wrapper to filter unimportant prints in silent mode
*/
void message(int prio, const char *format, ...)
{
    if (prio >= min_prio)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}
