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

bool parse_rule(char *string, ParsingContext *main_ctx, ParsingContext *extended_ctx, RewriteRule *out_rule)
{
    char *arrow_pos = strstr(string, ARROW);
    char *next_constr = strstr(string, WHERE); // Optional

    if (arrow_pos == NULL)
    {
        report_error("No arrow found.\n");
        return false;
    }

    // 1. Step: Overwrite "->" and "WHERE" with 0-terminators
    arrow_pos[0] = '\0';
    arrow_pos += strlen(ARROW);

    if (next_constr != NULL)
    {
        *next_constr = '\0';
        next_constr += strlen(WHERE);
    }

    size_t num_constrs = 0;
    Node *constrs[MATCHING_MAX_CONSTRAINTS];

    // 2. Parse contraints of form ".... WHERE x=y ; a=b"
    while (next_constr != NULL)
    {
        char *next_and = strstr(next_constr, AND);
        if (next_and != NULL)
        {
            *next_and = '\0';
            next_and += strlen(AND);
        }

        constrs[num_constrs] = parse_conveniently(extended_ctx, next_constr);
        num_constrs++;
        next_constr = next_and;
    }

    Node *left_n = parse_conveniently(main_ctx, string); // Gives error message
    if (left_n == NULL)
    {
        return false;
    }

    Node *right_n = parse_conveniently(main_ctx, arrow_pos);
    if (right_n == NULL)
    {
        free_tree(left_n);
        return false;
    }

    *out_rule = get_rule(get_pattern(left_n, num_constrs, constrs), right_n);
    return true;
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
            break;
        }
        if (curr_ruleset == (ssize_t)max_rulesets) break; // Buffer is full

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
