#include <string.h>
#include <stdio.h>

#include "rules.h"
#include "../util/alloc_wrappers.h"
#include "../util/console_util.h"

#define ARROW           " -> "
#define COMMENT_PREFIX  '\''

char *reduction_string =
"+x         -> x\n"
"$x         -> x\n"
"--x        -> x\n"
"x%         -> x/100\n"
"x/y        -> x*y^(-1)\n"
"log(x, e)  -> ln(x)\n"
"tan(x)     -> sin(x)/cos(x)\n"
"sqrt(x)    -> x^0.5\n"
"root(x, y) -> x^(1/y)";

char *derivation_string =
"deriv(cX, z)      -> 0\n"
"deriv(x, x)       -> 1\n"
"deriv(bX, z)      -> 0\n"
"deriv(cX*z, z)    -> cX\n"
"deriv(cX*z^cZ, z) -> cZ*cX*z^(cZ-1)\n"
"deriv(-x, z)      -> -deriv(x, z)\n"
"deriv(x + y, z)   -> deriv(x, z) + deriv(y, z)\n"
"deriv(x - y, z)   -> deriv(x, z) - deriv(y, z)\n"
"deriv(x*y, z)     -> deriv(x, z)*y + x*deriv(y, z)\n"
"deriv(x/y, z)     -> (deriv(x, z)*y - x*deriv(y, z)) / y^2\n"
"deriv(sin(x), z)  -> cos(x) * deriv(x, z)\n"
"deriv(cos(x), z)  -> -sin(x) * deriv(x, z)\n"
"deriv(tan(x), z)  -> deriv(x, z) * cos(x)^-2\n"
"deriv(e^y, z)     -> deriv(y, z)*e^y\n"
"deriv(x^y, z)     -> ((y*deriv(x, z))*x^-1 + deriv(y, z)*ln(x))*x^y\n"
"deriv(ln(x), z)   -> deriv(x, z)*^-1\n";

char *normal_form_string =
"x-y    -> x+(-y)\n"
"-(x+y) -> -x+(-y)\n"
"-(x*y) -> (-x)*y\n"
"dX+cY  -> cY+dX\n"
"dX*cY  -> cY*dX\n";

char *simplification_string =
// Get a nice sum
"x+y                        -> sum(x,y)\n"
"sum([xs], sum([ys]), [zs]) -> sum([xs], [ys], [zs])\n"
"sum([xs])+sum([ys])        -> sum([xs], [ys])\n"
"x+sum([xs])                -> sum(x, [xs])\n"
"sum([xs])+x                -> sum([xs], x)\n"

// Get a nice product
"x*y                          -> prod(x,y)\n"
"prod([xs], prod([ys]), [zs]) -> prod([xs], [ys], [zs])\n"
"prod([xs])*prod([ys])        -> prod([xs], [ys])\n"
"x*prod([xs])                 -> prod(x, [xs])\n"
"prod([xs])*x                 -> prod([xs], x)\n"

// Move constants and variables to the left
// Constants left to variables or operators
"sum([xs], dX, [ys], cY, [zs])  -> sum(cY, [xs], dX, [ys], [zs])\n"
// Variables left to operators
"sum([xs], oX, [ys], bY, [zs])  -> sum([xs], bY, [ys], oX, [zs])\n"
"prod([xs], dX, [ys], cY, [zs]) -> prod(cY, [xs], dX, [ys], [zs])\n"
"prod([xs], oX, [ys], bY, [zs]) -> prod([xs], bY, [ys], oX, [zs])\n"

// Simplify sums
"sum()                        -> 0\n"
"sum(x)                       -> x\n"
"sum([xs], 0, [ys])           -> sum([xs], [ys])\n"
"sum([xs], x, [ys], -x, [zs]) -> sum([xs], [ys], [zs])\n"
"sum([xs], -x, [ys], x, [zs]) -> sum([xs], [ys], [zs])\n"

// Simplify products
"prod()                           -> 1\n"
"prod(x)                          -> x\n"
"prod([xs], x, [ys], x, [zs])     -> prod([xs], x^2, [ys], [zs])\n"
"prod([xs], x, [ys], x^y, [zs])   -> prod([xs], x^(y+1), [ys], [zs])\n"
"prod([xs], x^y, [ys], x, [zs])   -> prod([xs], x^(y+1), [ys], [zs])\n"
"prod([xs], x^z, [ys], x^y, [zs]) -> prod([xs], x^(y+z), [ys], [zs])\n"
"prod([_], 0, [_])                -> 0\n"
"prod([xs], 1, [ys])              -> prod([xs], [ys])\n"
"prod([xs], -x, [ys])             -> -prod([xs], x, [ys])\n"

// Products within sum
"sum([xs], prod(a, x), [ys], x, [zs])                          -> sum([xs], prod(a+1, x), [ys], [zs])\n"
"sum([xs], prod(x, a), [ys], x, [zs])                          -> sum([xs], prod(x, a+1), [ys], [zs])\n"
"sum([xs], prod(a, x), [ys], -x, [zs])                          -> sum([xs], prod(a-1, x), [ys], [zs])\n"
"sum([xs], prod(x, a), [ys], -x, [zs])                          -> sum([xs], prod(x, a-1), [ys], [zs])\n"
"sum([xs], prod(b, x), [ys], prod(a, x), [zs])                 -> sum([xs], [ys], prod(a+b, x), [zs])\n"
"sum([xs], prod(x, b), [ys], prod(x, a), [zs])                 -> sum([xs], [ys], prod(a+b, x), [zs])\n"
"sum([xs], prod(b, x), [ys], prod(x, a), [zs])                 -> sum([xs], [ys], prod(a+b, x), [zs])\n"
"sum([xs], prod(x, b), [ys], prod(a, x), [zs])                 -> sum([xs], [ys], prod(a+b, x), [zs])\n"
"sum([xs], x, [ys], prod(a, x), [zs])                          -> sum([xs], [ys], prod(a+1, x), [zs])\n"
"sum([xs], x, [ys], prod(x, a), [zs])                          -> sum([xs], [ys], prod(x, a+1), [zs])\n"
"sum([xs], x, [ys], x, [zs])                                   -> sum(prod(2, x), [xs], [ys], [zs])\n"
"sum([xs], prod([yy], x, [zz]), [ys], prod([y], x, [z]), [zs]) -> sum([xs], x * (prod([yy],[zz]) + prod([y],[z])), [ys], [zs])\n"

// Powers
"(x^y)^z                          -> x^prod(y, z)\n"
"prod([xs], x^y, [ys], x^z, [zs]) -> prod([xs], x^(y+z), [ys], [zs])\n"
"prod([xs], y^x, [ys], z^x, [zs]) -> prod([xs], (y*z)^x, [ys], [zs])\n"
"x^0                              -> 1\n"
"prod(x, [xs])^z                  -> prod(x^z, prod([xs])^z)\n"

// Trigonometrics
"sum([xs], cos(x)^2, [ys], sin(x)^2) -> sum(1, [xs], [ys], [zs])\n"
"sum([xs], sin(x)^2, [ys], cos(x)^2) -> sum(1, [xs], [ys], [zs])\n";

char *pretty_string =
"prod([xs], sin(x), [ys], cos(x)^-1, [zs])   -> prod([xs], tan(x), [ys], [zs])\n"
"prod([xs], sin(x)^y, [ys], cos(x)^-y, [zs]) -> prod([xs], tan(x)^y, [ys], [zs])\n"

"-prod(x, [xs])   -> prod(-x, [xs])\n"
"-sum([xs], x)    -> -sum([xs])-x\n"
"prod([xs], x)^cY -> prod([xs])^cY * x^cY\n"

"(x+y)^2       -> x^2 + 2*x*y + y^2\n"
"cX*(x+y)      -> cX*x + cX*y\n"
"x+(y+z)       -> x+y+z\n"
"x*(y*z)       -> x*y*z\n"
"sum(x, y)     -> x+y\n"
"sum([xs], x)  -> sum([xs])+x\n"
"sum(x)        -> x\n"
"prod(x)       -> x\n"
"prod(x, y)    -> x*y\n"
"prod([xs], x) -> prod([xs])*x\n"
"--x           -> x\n"
"-1*x          -> -x\n"
"x+(-y)        -> x-y\n"
"x^1           -> x\n"
"root(x, 2)    -> sqrt(x)\n"
"x^(1/y)       -> root(x, y)\n";

bool parse_rule(char *string, ParsingContext *ctx, MappingFilter default_filter, Vector *ruleset)
{
    if (string[0] == COMMENT_PREFIX || string[0] == '\0')
    {
        return true;
    }

    char *right = strstr(string, ARROW);
    if (right == NULL)
    {
        return false;
    }

    right[0] = '\0';
    right += strlen(ARROW);
    Node *left_n = parse_conveniently(ctx, string);
    if (left_n == NULL)
    {
        return false;
    }

    Node *right_n = parse_conveniently(ctx, right);
    if (right_n == NULL)
    {
        free_tree(left_n);
        return false;
    }

    add_to_ruleset(ruleset, get_rule(left_n, right_n, default_filter));
    return true;
}

bool parse_ruleset_from_string(char *string, ParsingContext *ctx, MappingFilter default_filter, Vector *out_ruleset)
{
    // String is likely to be readonly - copy it
    char *copy = malloc_wrapper(strlen(string) + 1);
    strcpy(copy, string);

    *out_ruleset = get_empty_ruleset();
    size_t line_no = 0;
    char *line = copy;
    while (line != NULL)
    {
        line_no++;
        char *next_line = strstr(line, "\n");
        if (next_line != NULL)
        {
            next_line[0] = '\0';
        }

        if (!parse_rule(line, ctx, default_filter, out_ruleset))
        {
            software_defect("Failed parsing ruleset in line %zu.\n", line_no);
        }

        line = next_line;
        if (line != NULL) line++; // Skip newline char
    }

    vec_trim(out_ruleset);
    free(copy);
    return true;
}
