#include <string.h>
#include <stdio.h>

#include "rules.h"
#include "../util/alloc_wrappers.h"
#include "../util/console_util.h"

#define ARROW           " -> "


char *g_rulestrings[NUM_RULESETS] = {
// Operator elimination (de-sugar)
    "+x         -> x\n"
    "$x         -> x\n"
    "--x        -> x\n"
    "x%         -> x/100\n"
    "x/y        -> x*y^(-1)\n"
    "log(x, e)  -> ln(x)\n"
    "tan(x)     -> sin(x)/cos(x)\n"
    "sqrt(x)    -> x^0.5\n"
    "root(x, y) -> x^(1/y)",

// Deriv elimination
    "x*0               -> 0\n"
    "0*x               -> 0\n"
    "x+0               -> x\n"
    "0+x               -> x\n" // Smart but redundant elimination rules for performance
    "deriv(e, _)       -> 0\n"
    "deriv(pi, _)      -> 0\n"
    "deriv(cX, z)      -> 0\n"
    "deriv(x, x)       -> 1\n"
    "deriv(bX, z)      -> 0\n"
    "deriv(cX*z, z)    -> cX\n"
    "deriv(cX*z^cZ, z) -> cZ*cX*z^(cZ-1)\n"
    "deriv(-x, z)      -> -deriv(x, z)\n"
    "deriv(x + y, z)   -> deriv(x, z) + deriv(y, z)\n"
    "deriv(x - y, z)   -> deriv(x, z) - deriv(y, z)\n"
    "deriv(x*y, z)     -> deriv(x, z)*y + x*deriv(y, z)\n"
    "deriv(sin(x), z)  -> cos(x) * deriv(x, z)\n"
    "deriv(cos(x), z)  -> -sin(x) * deriv(x, z)\n"
    "deriv(tan(x), z)  -> deriv(x, z) * cos(x)^-2\n"
    "deriv(e^y, z)     -> deriv(y, z)*e^y\n"
    "deriv(x^y, z)     -> ((y*deriv(x, z))*x^-1 + deriv(y, z)*ln(x))*x^y\n"
    "deriv(ln(x), z)   -> deriv(x, z)*x^(-1)\n",

// Normal form
    "x-y    -> x+(-y)\n"
    "-(x+y) -> -x+(-y)\n"
    "-(x*y) -> (-x)*y\n"

    // Flatten products
    "x*y                          -> prod(x,y)\n"
    "prod([xs], prod([ys]), [zs]) -> prod([xs], [ys], [zs])\n"
    "prod([xs])*prod([ys])        -> prod([xs], [ys])\n"
    "x*prod([xs])                 -> prod(x, [xs])\n"
    "prod([xs])*x                 -> prod([xs], x)\n"

    // Flatten sums
    "x+y                        -> sum(x,y)\n"
    "sum([xs], sum([ys]), [zs]) -> sum([xs], [ys], [zs])\n"
    "sum([xs])+sum([ys])        -> sum([xs], [ys])\n"
    "x+sum([xs])                -> sum(x, [xs])\n"
    "sum([xs])+x                -> sum([xs], x)",

// Main simplification DONT USE INFIX + AND - HERE!
    // Flatten again
    "sum([xs], sum([ys]), [zs]) -> sum([xs], [ys], [zs])\n"
    "prod([xs], prod([ys]), [zs]) -> prod([xs], [ys], [zs])\n"

    // Misc.
    "--x -> x\n"
    // Fast elim
    "prod([_], 0, [_])            -> 0\n"
    "prod([xs], 1, [ys])          -> prod([xs], [ys])\n"
    // Fast elim
    "sum([xs], 0, [ys])         -> sum([xs], [ys])\n"
    // Simplify sums
    "sum()                        -> 0\n"
    "sum(x)                       -> x\n"
    "sum([xs], x, [ys], -x, [zs]) -> sum([xs], [ys], [zs])\n"
    "sum([xs], -x, [ys], x, [zs]) -> sum([xs], [ys], [zs])\n"
    // Simplify products
    "prod()                           -> 1\n"
    "prod(x)                          -> x\n"
    "prod(z,[xs],-x,[ys])             -> prod(-z,[xs],x,[ys])\n"
    "prod([xs], x, [ys], x, [zs])     -> prod([xs], x^2, [ys], [zs])\n"
    "prod([xs], x, [ys], x^y, [zs])   -> prod([xs], x^sum(y,1), [ys], [zs])\n"
    "prod([xs], x^y, [ys], x, [zs])   -> prod([xs], x^sum(y,1), [ys], [zs])\n"
    "prod([xs], x^z, [ys], x^y, [zs]) -> prod([xs], x^sum(y,z), [ys], [zs])\n"
    "prod([xs], -1, [ys])             -> -prod([xs], [ys])\n"
    "prod(x,[xs])^y                   -> prod(x^y, prod([xs])^y)\n"
    // Products within sum
    "sum([xs], prod(a, x), [ys], x, [zs])                          -> sum([xs], prod(sum(a,1), x), [ys], [zs])\n"
    "sum([xs], prod(x, a), [ys], x, [zs])                          -> sum([xs], prod(x, sum(a,1)), [ys], [zs])\n"
    "sum([xs], prod(a, x), [ys], -x, [zs])                         -> sum([xs], prod(sum(a,-1), x), [ys], [zs])\n"
    "sum([xs], prod(x, a), [ys], -x, [zs])                         -> sum([xs], prod(x, sum(a,-1)), [ys], [zs])\n"
    "sum([xs], prod(b, x), [ys], prod(a, x), [zs])                 -> sum([xs], [ys], prod(sum(b,a), x), [zs])\n"
    "sum([xs], prod(x, b), [ys], prod(x, a), [zs])                 -> sum([xs], [ys], prod(x, sum(b,a)), [zs])\n"
    "sum([xs], prod(b, x), [ys], prod(x, a), [zs])                 -> sum([xs], [ys], prod(sum(b,a), x), [zs])\n"
    "sum([xs], prod(x, b), [ys], prod(a, x), [zs])                 -> sum([xs], [ys], prod(sum(b,a), x), [zs])\n"
    "sum([xs], x, [ys], prod(a, x), [zs])                          -> sum([xs], [ys], prod(sum(1,a), x), [zs])\n"
    "sum([xs], x, [ys], prod(x, a), [zs])                          -> sum([xs], [ys], prod(x, sum(a,1)), [zs])\n"
    "sum([xs], x, [ys], x, [zs])                                   -> sum(prod(2, x), [xs], [ys], [zs])\n"
    // Powers
    "x^1                              -> x\n"
    "(x^y)^z                          -> x^prod(y, z)\n"
    "prod([xs], x^y, [ys], x^z, [zs]) -> prod([xs], x^sum(y,z), [ys], [zs])\n"
    //"prod([xs], y^x, [ys], z^x, [zs]) -> prod([xs], (y*z)^x, [ys], [zs])\n"
    "x^0                              -> 1\n"
    "prod(x, [xs])^z                  -> prod(x^z, prod([xs])^z)\n"
    // Trigonometrics
    "sum([xs], cos(x)^2, [ys], sin(x)^2)         -> sum(1, [xs], [ys], [zs])\n"
    "sum([xs], sin(x)^2, [ys], cos(x)^2)         -> sum(1, [xs], [ys], [zs])\n"
    "prod([xs], sin(x), [ys], cos(x)^-1, [zs])   -> prod([xs], tan(x), [ys], [zs])\n"
    "prod([xs], sin(x)^y, [ys], cos(x)^-y, [zs]) -> prod([xs], tan(x)^y, [ys], [zs])\n"
    // Some special ops
    "min(x, x)     -> x\n"
    "max(x, x)     -> x\n"
    // Ordering trick: Move constants to the left and fold them again in a + or * term
    // Since this ruleset will not affect + and *, the term will be evaluated eventually
    "sum([xs],cX,[ys],cY,[zs])  -> sum(cX+cY,[xs],[ys],[zs])\n"
    "sum(x+y,[xs],cX,[ys])      -> sum(x+y+cX,[xs],[ys])\n"
    "sum(x,[xs],cX,[ys])        -> sum(cX,x,[xs],[ys])\n"
    "prod([xs],cX,[ys],cY,[zs]) -> prod(cX*cY,[xs],[ys],[zs])\n"
    "prod(x*y,[xs],cX,[ys])     -> prod(x*y*cX,[xs],[ys])\n"
    "prod(x,[xs],cX,[ys])       -> prod(cX,x,[xs],[ys])",

// Fold flattened operators back
    "-sum([xs], x)                           -> -sum([xs])-x\n"
    "sum([xs], x)                            -> sum([xs])+x\n"
    "sum(x)                                  -> x\n"
    "prod([xs], x)                           -> prod([xs])*x\n"
    "prod(x)                                 -> x\n"
    "sum([xs], prod([xxs], -x, [yys]), [ys]) -> sum([xs], [ys]) - prod([xxs],x,[yys])\n"
    "prod([xs], x)^cY                        -> prod([xs])^cY * x^cY\n",
    "prod()                                  -> 1\n"
    "sum()                                   -> 0"

// Make output pretty
    "0*x           -> 0\n"
    "1*x           -> x\n"
    "0+x           -> x\n"
    "x^(-1)        -> 1/x\n"
    "x^1           -> x\n"

    // Output ordering: Constants to the front
    "dX*cX         -> cX*dX\n"
    "dX+cX         -> cX+dX\n"
    "dX*-cX        -> -cX*dX\n"
    "dX+cX         -> cX+dX\n"

    "x*(cY/z)      -> (cY*x)/z\n"
    "x+((-y)/z)    -> x-(y/z)\n"
    "x+(-y*z)      -> x-y*z\n"
    "(x+y)^2       -> x^2 + 2*x*y + y^2\n"
    "cX*(x+y)      -> cX*x + cX*y\n"
    "x+(y+z)       -> x+y+z\n"
    "x*(y*z)       -> x*y*z\n"
    "--x           -> x\n"
    "x*(-1)        -> -x\n"
    "(-1)*x        -> -x\n"
    "x+(-y)        -> x-y\n"
    "-(x*y)        -> (-x)*y\n"
    "root(x, 2)    -> sqrt(x)\n"
    "x^(1/y)       -> root(x, y)"
};

bool parse_rule(char *string, ParsingContext *ctx, MappingFilter default_filter, Vector *ruleset)
{
    if (string[0] == '\0')
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
