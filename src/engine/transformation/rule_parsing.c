/*
Todo: Refactor, especially string copying due to unknown source of string
Could be heap but could also be string literal that is readonly
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

#include "../util/alloc_wrappers.h"
#include "../util/console_util.h"
#include "../util/string_util.h"
#include "../tree/node.h"
#include "../parsing/parser.h"
#include "rule_parsing.h"
#include "matching.h"

#define COMMENT_PREFIX "#"
#define RULESET        "RULESET"
#define ARROW          "->"
#define WHERE          " WHERE "
#define AND            " ; "

void report_pattern_errcode(int err_code)
{
    switch (err_code)
    {
        case -1:
            report_error("Trying to preprocess a pattern with too many distinct variables. Increase MAX_MAPPED_VARS.\n");
            return;
        case -2:
            report_error("Trying to preprocess a pattern with too many variable occurrances. Increase MAX_VARIABLE_OCCURRANCES.\n");
            return;
        case -3:
            report_error("Trying to preprocess a pattern with too many constraints. Increase MATCHING_MAX_CONSTRAINTS.\n");
    }
}

// string: without "WHERE"
bool parse_constraints(const char *string,
    const ParsingContext *ctx,
    size_t *num_constraints, // In-Out, like in getline
    Node **out_constraints)
{
    if (string == NULL)
    {
        *num_constraints = 0;
        return true;
    }

    char *str_cpy = malloc_wrapper(strlen(string) + 1);
    char *str = str_cpy;
    strcpy(str, string);    

    size_t buffer_size = *num_constraints;
    *num_constraints = 0;
    while (str != NULL)
    {
        char *next_and = strstr(str, AND);
        if (next_and != NULL)
        {
            *next_and = '\0';
            next_and += strlen(AND);
        }

        out_constraints[*num_constraints] = parse_conveniently(ctx, str);
        if (out_constraints[*num_constraints] == NULL) goto error;
        (*num_constraints)++;
        if (*num_constraints == buffer_size) goto success;
        str = next_and;
    }

    success:
    free(str_cpy);
    return true;
    error:
    free(str_cpy);
    return false;
}

// string: <expr> WHERE <expr> ; <expr>... (i.e. without '-> after')
bool parse_pattern(const char *string, const ParsingContext *ctx, Pattern *out_pattern)
{
    char *str_cpy = malloc_wrapper(strlen(string) + 1);
    char *str = str_cpy;
    strcpy(str_cpy, string);

    char *where_pos = strstr(str, WHERE); // Optional
    size_t num_constrs = MATCHING_MAX_CONSTRAINTS;
    Node *constrs[MATCHING_MAX_CONSTRAINTS];
    if (where_pos != NULL)
    {
        *where_pos = '\0';
        where_pos += strlen(WHERE);
    }
    if (!parse_constraints(where_pos, ctx, &num_constrs, constrs))
    {
        goto error;
    }

    Node *pattern = parse_conveniently(ctx, str); // Gives error message
    if (pattern == NULL)
    {
        goto error;
    }

    int err_code = get_pattern(pattern, num_constrs, constrs, out_pattern);
    if (err_code != 0)
    {
        report_pattern_errcode(err_code);
        goto error;
    }
    free(str_cpy);
    return true;

    error:
    free(str_cpy);
    return false;
}

bool parse_rule(const char *string, const ParsingContext *ctx, RewriteRule *out_rule)
{
    char *str = malloc_wrapper(strlen(string) + 1);
    strcpy(str, string);

    char *arrow_pos = strstr(str, ARROW);
    char *where_pos = strstr(str, WHERE); // Optional
    Node *left_n = NULL;
    Node *right_n = NULL;
    size_t num_constrs = 0;

    if (arrow_pos == NULL)
    {
        report_error("No arrow found\n");
        goto error;
    }

    // 1. Step: Overwrite "->" and "WHERE" with 0-terminators
    arrow_pos[0] = '\0';
    arrow_pos += strlen(ARROW);

    Node *constrs[MATCHING_MAX_CONSTRAINTS];
    if (where_pos != NULL)
    {
        *where_pos = '\0';
        where_pos += strlen(WHERE);
    }

    num_constrs = MATCHING_MAX_CONSTRAINTS;
    if (!parse_constraints(where_pos, ctx, &num_constrs, constrs)) goto error;

    left_n = parse_conveniently(ctx, str); // Gives error message
    if (left_n == NULL)
    {
        goto error;
    }

    right_n = parse_conveniently(ctx, arrow_pos);
    if (right_n == NULL)
    {
        goto error;
    }

    Pattern pattern;
    int err_code = get_pattern(left_n, num_constrs, constrs, &pattern);
    if (err_code != 0)
    {
        report_pattern_errcode(err_code);
        goto error;
    }
    if (!get_rule(pattern, right_n, out_rule))
    {
        report_error("Unbounded variable in righthand side of rule.\n");
        goto error;
    }

    free(str);
    return true;

    error:
    free(str);
    free_tree(left_n);
    free_tree(right_n);
    for (size_t i = 0; i < num_constrs; i++) free_tree(constrs[i]);
    return false;
}

ssize_t parse_rulesets_from_file(FILE *file,
    const ParsingContext *ctx,
    size_t buffer_size,
    Vector *out_rulesets)
{
    ssize_t curr_ruleset = -1;
    size_t line_index = 0;
    char *line_start = NULL;
    size_t line_length = 0;

    while (getline(&line_start, &line_length, file) != EOF)
    {
        line_index++;
        char *line = line_start;

        line[strlen(line) - 1] = '\0'; // Overwrite newline char
        char *comment_start = strstr(line, COMMENT_PREFIX);
        if (comment_start != NULL) *comment_start = '\0'; // Prune comments from the right
        while (*line == ' ') line++; // Strip leading spaces

        if (begins_with(RULESET, line))
        {
            curr_ruleset++;
            continue;
        }

        if (curr_ruleset == (ssize_t)buffer_size)
        {
            curr_ruleset--; // Adjusted for right return value
            break;          // Buffer is full
        }

        if (line[0] == '\0') continue;

        if (curr_ruleset != -1)
        {
            if (!parse_rule(line, ctx, vec_push_empty(&out_rulesets[curr_ruleset])))
            {
                report_error("Error occurred in line %zu.\n", line_index);
                goto error;
            }
        }
        else
        {
            report_error("Ignored non-comment line before first ruleset.\n");
        }
    }

    free(line_start);
    return curr_ruleset + 1;

    error:
    free(line_start);
    return -1;
}
