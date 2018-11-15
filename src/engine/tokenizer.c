#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "console_util.h"

bool add_token(char **tokens, char *position, int *num_tokens)
{
    if (*num_tokens == MAX_TOKENS) return false;
    tokens[(*num_tokens)++] = position;
    return true;
}

// Sorts strings in descending length
int cmpfunc(const void *a, const void *b)
{
    size_t len_a = strlen(*(char**)a);
    size_t len_b = strlen(*(char**)b);
    if (len_a == len_b) return 0;
    if (len_a > len_b) return -1;
    return 1;
}

/*
Summary: Splits input string into several tokens to be parsed
Returns: True if method succeeded, False if MAX_TOKENS was exceeded
*/
bool tokenize(ParsingContext *ctx, char *input, char *out_tokens[MAX_TOKENS], int *out_num_tokens)
{
    char *token_markers[MAX_TOKENS];
    char *keywords[ctx->num_ops];
    for (int i = 0; i < ctx->num_ops; i++)
    {
        keywords[i] = ctx->operators[i].name;
    }
    // Maximal munch
    qsort(keywords, ctx->num_ops, sizeof(char*), &cmpfunc);
    
    int state = 0; // 0: initial, 1: default, 2: letter, 3: digit, 4: keyword
    int num_markers = 0;
    
    if (!add_token(token_markers, input, &num_markers)) return false;

    for (char *current = input; *current != '\0'; current++)
    {
        int next_state = 1;
        bool token = false;
        
        if (is_digit(*current))
        {
            next_state = 3;
        }
        else
        {
            if (is_letter(*current))
            {
                next_state = 2;
            }
        }
    
        // Did we find a keyword?
        if (next_state != 2) // We don't want to find keywords in strings
        {
            for (size_t j = 0; j < ctx->num_ops; j++)
            {
                size_t keyw_len = strlen(keywords[j]);
                if (keyw_len == 0) continue;
                
                if (begins_with(keywords[j], current))
                {
                    if (state != 0 && !add_token(token_markers, current, &num_markers)) return false;
                    current += keyw_len - 1;
                    token = true;
                    next_state = 4;
                    break;
                }
            }
        }
        
        // Did we find a special char or ended a digit or string?
        if (!token)
        {
            if (state != 0 && (next_state == 1 || state == 4 || next_state != state ))
            {
                if (!add_token(token_markers, current, &num_markers)) return false;
            }
        }
        
        state = next_state;
    }
    
    if (!add_token(token_markers, input + strlen(input), &num_markers)) return false; // Sentinel marker at \0
    
    // Build \0-terminated strings from pointers and out_num_tokens
    // (might be less than num_markers due to whitespace-tokens that are removed)
    *out_num_tokens = 0;
    for (int i = 0; i < num_markers - 1; i++)
    {
        // Check for whitespace-token and empty token (only when input itself is empty)
        if (is_space(token_markers[i][0]) || token_markers[i] == token_markers[i + 1]) continue;
        
        int tok_len = (int)(token_markers[i + 1] - token_markers[i]);
        out_tokens[*out_num_tokens] = malloc(tok_len + 1);

        for (int j = 0; j < tok_len; j++)
        {
            out_tokens[*out_num_tokens][j] = token_markers[i][j];
        }

        out_tokens[*out_num_tokens][tok_len] = '\0';
        (*out_num_tokens)++;
    }
    return true;
}

bool is_space(char c)
{
    return c == ' ' || c == '\t';
}

bool is_digit(char c)
{
    return (c >= '0' && c <= '9') || c == '.';
}

bool is_opening_parenthesis(char c)
{
    return c == '(' || c == '[' || c == '{';
}

bool is_closing_parenthesis(char c)
{
    return c == ')' || c == ']' || c == '}';
}

bool is_letter(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

bool is_delimiter(char c)
{
    return c == ',' || c == ';';
}
