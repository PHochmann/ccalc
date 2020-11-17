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

// string: without "WHERE"
bool parse_constraints(const char *string,
    ParsingContext *extended_ctx,
    size_t *num_constraints, // In-Out, like in readline
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

        out_constraints[*num_constraints] = parse_conveniently(extended_ctx, str);
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

bool parse_pattern(const char *string, ParsingContext *main_ctx, ParsingContext *extended_ctx, Pattern *out_pattern)
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
    if (!parse_constraints(where_pos, extended_ctx, &num_constrs, constrs))
    {
        goto error;
    }

    Node *pattern = parse_conveniently(main_ctx, str); // Gives error message
    if (pattern == NULL)
    {
        goto error;
    }

    *out_pattern = get_pattern(pattern, num_constrs, constrs);
    free(str_cpy);
    return true;

    error:
    free(str_cpy);
    return false;
}

bool parse_rule(const char *string, ParsingContext *main_ctx, ParsingContext *extended_ctx, RewriteRule *out_rule)
{
    char *str = malloc_wrapper(strlen(string) + 1);
    strcpy(str, string);

    char *arrow_pos = strstr(str, ARROW);
    char *where_pos = strstr(str, WHERE); // Optional
    Node *left_n = NULL;
    Node *right_n = NULL;

    if (arrow_pos == NULL)
    {
        report_error("No arrow found.\n");
        goto error;
    }

    // 1. Step: Overwrite "->" and "WHERE" with 0-terminators
    arrow_pos[0] = '\0';
    arrow_pos += strlen(ARROW);

    size_t num_constrs = MATCHING_MAX_CONSTRAINTS;
    Node *constrs[MATCHING_MAX_CONSTRAINTS];
    if (where_pos != NULL)
    {
        *where_pos = '\0';
        where_pos += strlen(WHERE);
    }
    if (!parse_constraints(where_pos, extended_ctx, &num_constrs, constrs)) goto error;

    left_n = parse_conveniently(main_ctx, str); // Gives error message
    if (left_n == NULL)
    {
        goto error;
    }

    right_n = parse_conveniently(main_ctx, arrow_pos);
    if (right_n == NULL)
    {
        goto error;
    }

    // Error message given by get_rule
    if (!get_rule(get_pattern(left_n, num_constrs, constrs), right_n, out_rule))
    {
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

size_t parse_rulesets_from_file(FILE *file,
    ParsingContext *main_ctx,
    ParsingContext *extended_ctx,
    size_t max_rulesets,
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
        while (*line == ' ') line++;   // Strip leading spaces

        if (begins_with(RULESET, line))
        {
            curr_ruleset++;
            continue;
        }

        if (curr_ruleset == (ssize_t)max_rulesets)
        {
            break; // Buffer is full
        }

        if (begins_with(COMMENT_PREFIX, line)) continue;
        if (line[0] == '\0') continue;

        if (curr_ruleset == -1)
        {
            report_error("Ignored non-comment line outside of ruleset.\n");
        }

        if (!parse_rule(line, main_ctx, extended_ctx, vec_push_empty(&out_rulesets[curr_ruleset])))
        {
            report_error("Error occurred in line %zu.\n", line_index);
        }
    }

    free(line_start);
    return curr_ruleset + 1;
}
