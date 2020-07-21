#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

#include "string_util.h"

#define ESC_START  27
#define ESC_END   109

bool is_space(char c)
{
    return c == ' ';
}

bool is_digit(char c)
{
    return (c >= '0' && c <= '9') || c == '.';
}

bool is_letter(char c)
{
    return (c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z')
        || c == '_'
        || c == '['
        || c == ']';
}

bool is_opening_parenthesis(char *c)
{
    return strcmp(c, "(") == 0 || strcmp(c, "{") == 0;
}

bool is_closing_parenthesis(char *c)
{
    return strcmp(c, ")") == 0 || strcmp(c, "}") == 0;
}

bool is_delimiter(char *c)
{
    return strcmp(c, ",") == 0;
}

bool begins_with(char *prefix, char *str)
{
    size_t prefix_length = strlen(prefix);
    size_t string_length = strlen(str);
    if (prefix_length > string_length) return false;
    return strncmp(prefix, str, prefix_length) == 0;
}

/*
Summary: Helper function when parsing commands to extract strings between delimiters
    Currently only used in cmd_table_exec
Example: split("abc x def ghi", out, 2, " x ", " g") = 3
    out = { "abc", "def", "hi" }
*/
size_t str_split(char *str, char **out_strs, size_t num_delimiters, ...)
{
    va_list args;
    va_start(args, num_delimiters);
    size_t res = 1;
    for (size_t i = 0; i < num_delimiters; i++)
    {
        char *delimiter = va_arg(args, char*);
        char *end_pos = strstr(str, delimiter);

        if (end_pos != NULL)
        {
            *end_pos = '\0';
            out_strs[res - 1] = str;
            res++;
            str = end_pos + strlen(delimiter);
        }
        else
        {
            break;
        }
    }

    out_strs[res - 1] = str;
    return res;
}

/*
Returns: String representation of ParserError
*/
char *perr_to_string(ParserError perr)
{
    switch (perr)
    {
        case PERR_SUCCESS:
            return "Success";
        case PERR_UNEXPECTED_SUBEXPRESSION:
            return "Unexpected subexpression";
        case PERR_EXCESS_OPENING_PARENTHESIS:
            return "Missing closing parenthesis";
        case PERR_EXCESS_CLOSING_PARENTHESIS:
            return "Unexpected closing parenthesis";
        case PERR_UNEXPECTED_DELIMITER:
            return "Unexpected delimiter";
        case PERR_MISSING_OPERATOR:
            return "Unexpected operand";
        case PERR_MISSING_OPERAND:
            return "Missing operand";
        case PERR_OUT_OF_MEMORY:
            return "Out of memory";
        case PERR_FUNCTION_WRONG_ARITY:
            return "Wrong number of operands of function";
        case PERR_CHILDREN_EXCEEDED:
            return "Exceeded maximum number of operands of function";
        case PERR_EMPTY:
            return "Empty Expression";
        default:
            return "Unknown Error";
    }
}

StringIterator get_iterator(char *string)
{
    return (StringIterator){ .string = string, .index = 0 };
}

char get_next_char(StringIterator *iterator)
{
    if (iterator->string[iterator->index] == ESC_START)
    {
        while (iterator->string[iterator->index] != ESC_END)
        {
            iterator->index++;
        }
        iterator->index++;
    }
    return iterator->string[iterator->index++];
}

/*
Summary: Calculates length of string displayed in console,
    i.e. reads until \0 or \n and omits ANSI-escaped color sequences
    Todo: Consider \t and other special chars
*/
size_t console_strlen(char *str)
{
    if (str == NULL) return 0;
    StringIterator iterator = get_iterator(str);
    size_t res = 0;
    while (true)
    {
        char curr = get_next_char(&iterator);
        if (curr == '\0' || curr == '\n') break;
        res++;
    }
    return res;
}

// Stringbuilder follows ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

Vector strbuilder_create(size_t start_size)
{
    Vector builder = vec_create(sizeof(char), start_size);
    *(char*)vec_push_empty(&builder) = '\0';
    return builder;
}

void strbuilder_reset(Vector *builder)
{
    vec_reset(builder),
    *(char*)vec_push_empty(builder) = '\0';
}

void strbuilder_append(Vector *builder, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vstrbuilder_append(builder, fmt, args);
    va_end(args);
}

void strbuilder_append_char(Vector *builder, char c)
{
    VEC_SET_ELEM(builder, char, builder->elem_count - 1, c);
    VEC_PUSH_ELEM(builder, char, '\0');
}

void strbuilder_reverse(Vector *builder)
{
    for (size_t i = 0; i < (vec_count(builder) - 1) / 2; i++)
    {
        char temp = VEC_GET_ELEM(builder, char, i);
        size_t partner = vec_count(builder) - 2 - i;
        VEC_SET_ELEM(builder, char, i, VEC_GET_ELEM(builder, char, partner));
        VEC_SET_ELEM(builder, char, partner, temp);
    }
}

void vstrbuilder_append(Vector *builder, char *fmt, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    size_t appended_length = vsnprintf(vec_get(builder, vec_count(builder) - 1),
        builder->buffer_size - builder->elem_count + 1,
        fmt, args);

    if (vec_ensure_size(builder, vec_count(builder) + appended_length))
    {
        vsnprintf(vec_get(builder, vec_count(builder) - 1),
            builder->buffer_size - builder->elem_count + 1,
            fmt, args_copy);
    }
    builder->elem_count += appended_length;
    va_end(args_copy);
}

// Trie follows ---

#define START_CHAR '!'
#define END_CHAR   'z'

typedef struct TrieNode
{
    bool is_leaf;
    size_t next[END_CHAR - START_CHAR + 1];
} TrieNode;

TrieNode get_empty_trienode()
{
    TrieNode res;
    res.is_leaf = false;
    for (size_t i = 0; i < END_CHAR - START_CHAR + 1; i++)
    {
        res.next[i] = SIZE_MAX;
    }
    return res;
}

Vector trie_create()
{
    Vector res = vec_create(sizeof(TrieNode), 1);
    VEC_PUSH_ELEM(&res, TrieNode, get_empty_trienode());
    return res;
}

void trie_add_str(Vector *trie, char *string)
{
    size_t curr_index = 0;
    for (size_t i = 0; string[i] != '\0'; i++)
    {
        if (string[i] < START_CHAR || string[i] > END_CHAR)
        {
            return;
        }

        unsigned char index = string[i] - START_CHAR;
        if (VEC_GET_ELEM(trie, TrieNode, curr_index).next[index] == SIZE_MAX)
        {
            VEC_PUSH_ELEM(trie, TrieNode, get_empty_trienode());
            VEC_GET_ELEM(trie, TrieNode, curr_index).next[index] = vec_count(trie) - 1;
        }
        curr_index = VEC_GET_ELEM(trie, TrieNode, curr_index).next[index];
    }
    VEC_GET_ELEM(trie, TrieNode, curr_index).is_leaf = true;
}

bool trie_contains(Vector *trie, char *string)
{
    return trie_longest_prefix(trie, string) == strlen(string);
}

size_t trie_longest_prefix(Vector *trie, char *string)
{
    size_t res = 0;
    TrieNode *curr = vec_get(trie, 0);

    for (size_t i = 0; string[i] != '\0'; i++)
    {
        // Char in string that can not be in trie
        if (string[i] < START_CHAR || string[i] > END_CHAR) return res;

        unsigned char index = string[i] - START_CHAR;
        if (curr->next[index] != SIZE_MAX)
        {
            curr = vec_get(trie, curr->next[index]);
        }
        else
        {
            return res;
        }

        if (curr->is_leaf) res = i + 1;
    }

    return 0;
}
