#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "../util/string_util.h"
#include "../util/trie.h"
#include "../util/alloc_wrappers.h"
#include "tokenizer.h"

#define VECTOR_STARTSIZE 10

typedef enum
{
    TOKSTATE_NEW,
    TOKSTATE_LETTER,
    TOKSTATE_DIGIT,
    TOKSTATE_OTHER,
} TokState;

static void push_token(const char *input, size_t length, Vector *tokens)
{
    if (length == 0) return;
    char *tok = malloc_wrapper(length + 1);
    for (size_t i = 0; i < length; i++)
    {
        tok[i] = input[i];
    }
    tok[length] = '\0';
    VEC_PUSH_ELEM(tokens, char*, tok);
}

/*
Summary: Splits input string into several tokens to be parsed
Params:
    input:         Input string to tokenize
    keywords_trie: Allowed to be NULL
    out_tokens:    Vector of pointers to malloced tokens (free with free_tokens)
*/
Vector tokenize(const char *input, const Trie *keywords_trie)
{
    Vector res = vec_create(sizeof(char*), VECTOR_STARTSIZE);
    TokState state = TOKSTATE_NEW;
    
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
    
        // Did the current token end?
        if (state != TOKSTATE_NEW && (next_state != state || next_state == TOKSTATE_OTHER))
        {
            push_token(input + next_token_start, i - next_token_start, &res);
            next_token_start = i;
        }

        // Did we find a keyword?
        if (next_state != TOKSTATE_LETTER && keywords_trie != NULL) // We don't want to find keywords in strings
        {
            size_t keyword_len = trie_longest_prefix(keywords_trie, input + i, NULL);
            if (keyword_len > 0)
            {
                push_token(input + i, keyword_len, &res);
                next_token_start += keyword_len;
                i += keyword_len - 1;
                next_state = TOKSTATE_NEW;
            }
        }
        
        state = next_state;
    }

    push_token(input + next_token_start, strlen(input) - next_token_start, &res);
    return res;
}

void free_tokens(Vector *tokens)
{
    if (tokens == NULL) return;
    for (size_t i = 0; i < vec_count(tokens); i++)
    {
        free(*(char**)vec_get(tokens, i));
    }
    vec_destroy(tokens);
}
