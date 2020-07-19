#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "../util/string_util.h"
#include "tokenizer.h"

#define VECTOR_STARTSIZE 5

typedef enum
{
    TOKSTATE_NEW,
    TOKSTATE_LETTER,
    TOKSTATE_DIGIT,
    TOKSTATE_OTHER,
} TokState;

// Sorts strings in descending length
int strcmp_by_length(const void *a, const void *b)
{
    size_t len_a = strlen(*(char**)a);
    size_t len_b = strlen(*(char**)b);
    if (len_a == len_b) return 0;
    if (len_a > len_b) return -1;
    return 1;
}

void push_token(char *input, size_t start, size_t length, Vector *tokens)
{
    if (length == 0) return;
    char *tok = malloc(length + 1);
    for (size_t i = 0; i < length; i++)
    {
        tok[i] = input[start + i];
    }
    tok[length] = '\0';
    VEC_PUSH_ELEM(tokens, char*, tok);
}

/*
Summary: Splits input string into several tokens to be parsed
*/
void tokenize(ParsingContext *ctx, char *input, Vector *out_tokens)
{
    if (ctx == NULL || input == NULL) return;

    char *keywords[ctx->num_ops];
    *out_tokens = vec_create(sizeof(char*), VECTOR_STARTSIZE);
    TokState state = TOKSTATE_NEW;

    for (size_t i = 0; i < ctx->num_ops; i++)
    {
        keywords[i] = ctx->operators[i].name;
    }

    // Maximal munch
    qsort(keywords, ctx->num_ops, sizeof(char*), &strcmp_by_length);
    
    size_t next_token_start = 0;
    for (size_t i = 0; input[i] != '\0'; i++)
    {
        TokState next_state = TOKSTATE_OTHER;
        
        if (is_digit(input[i]))
        {
            next_state = TOKSTATE_DIGIT;
        }
        else
        {
            if (is_letter(input[i]))
            {
                next_state = TOKSTATE_LETTER;
            }
        }
    
        // Did we find a special char or ended a digit or string?
        if (state != TOKSTATE_NEW && (next_state != state || next_state == TOKSTATE_OTHER))
        {
            push_token(input, next_token_start, i - next_token_start, out_tokens);
            next_token_start = i;
        }

        if (is_space(input[i]))
        {
            next_token_start++;
            continue;
        }

        // Did we find a keyword?
        if (next_state != TOKSTATE_LETTER) // We don't want to find keywords in strings
        {
            for (size_t j = 0; j < ctx->num_ops; j++)
            {
                if (begins_with(keywords[j], input + i))
                {
                    size_t keyword_length = strlen(keywords[j]);
                    push_token(input, i, keyword_length, out_tokens);
                    next_token_start += keyword_length;
                    i += keyword_length - 1;
                    next_state = TOKSTATE_NEW;
                    break;
                }
            }
        }
        
        state = next_state;
    }

    push_token(input, next_token_start, strlen(input) - next_token_start, out_tokens);
}

void free_tokens(Vector *tokens)
{
    for (size_t i = 0; i < vec_count(tokens); i++)
    {
        free(VEC_GET_ELEM(tokens, char*, i));
    }
    vec_destroy(tokens);
}
