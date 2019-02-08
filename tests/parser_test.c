#include <stdio.h>
#include <math.h>

#include "../src/engine/constants.h"
#include "../src/engine/context.h"
#include "../src/engine/node.h"
#include "../src/engine/parser.h"
#include "../src/commands/arith_context.h"

#define PAD_PARENTHESES true
#define EPSILON 0.00000001
#define NUM_VALUE_TESTS 67
#define NUM_ERROR_TESTS 7

// To check if parsed tree evaluates to expected value
typedef struct {
    char *input;
    double result; 
} ValueTest;

// To check if parser returns expected error on malformed inputs
typedef struct {
    char *input;
    ParserError result;
} ErrorTest;

static ValueTest valueTests[NUM_VALUE_TESTS] = {
    { "2+3", 5 },
    { "2-3", -1 },
    { "2*3", 6 },
    { "4/2", 2 },
    { "2^3", 8 },
    { "8 C 2", 28 },
    { "20 mod 15", 5 },
    { "-3", -3 },
    { "+99", 99 },
    { "4!", 24 },
    { "3%", 0.03 },
    { "exp(2)", 7.3890560989 },
    { "root(27, 3)", 3 },
    { "sqrt(36)", 6 },
    { "log(128, 2)", 7 },
    { "ln(e)", 1 },
    { "ld(512)", 9 },
    { "lg(10)", 1 },
    { "sin(pi)", 0 },
    { "cos(pi)", -1 },
    { "tan(pi)", 0 },
    { "asin(0)", 0 },
    { "acos(1)", 0 },
    { "atan(0)", 0 },
    { "sinh(1)", 1.1752011936438 },
    { "cosh(1)", 1.5430806348152 },
    { "tanh(1)", 0.7615941559557 },
    { "asinh(0.5)", 0.4812118250596034 },
    { "acosh(2)", 1.316957896924816708 },
    { "atanh(0.5)", 0.5493061443340548 },
    { "max(1, 2, 3)", 3 },
    { "min(1, 2, 3)", 1 },
    { "abs(-2)", 2 },
    { "ceil(4.5)", 5 },
    { "floor(-4.5)", -5 },
    { "round(.5)", 1 },
    { "trunc(-.5)", 0 },
    { "frac(-5.5)", 0.5 },
    { "sum(1, 2, 3)", 6 },
    { "prod(3, 4, 5)", 60 },
    { "avg(0, 4)", 2 },
    { "gamma(2)", 1 },
    { "pi", 3.141592653 },
    { "e", 2.7182818284 },
    { "phi", 1.61803398 },

    { "1+2*3+4", 11 },
    { " ( 9.0 *  0)", 0 },
    { "sin(0)", 0 },
    { "sqrt2", 1.4142135623 },
    { "sin0+2", 2 },
    { "(cos sin.123pi.123)", 0.38351121094 },
    { "avg()", 0 },
    { "1+sum()*2", 1 },
    { "sum(1; 2, 3)*2", 12 },
    { "-pi e", -8.5397342226 },
    { "((--1)) sum2 !%", 0.02 },
    { "--(1+sum(ld--8, --1%+--1%, 2 .2))%+1", 1.0442 },
    { "2^2^3", 256 },
    { "(2^2)^3", 64 },
    { "2^2^3 - 2^(2^3)", 0 },
    { "1/2/2", 0.25 },
    { "1/[{2/2]}", 1 },
    { "2*3^2", 18 },
    { "sin(asin(.2))", 0.2 },
    { "-sqrt(abs(--2!!*--sum(-1+.2-.2+2, 2^2^3-255, -sum(.1, .9), 1+2)*--2!!))", -4 },
#if PAD_PARENTHESES
    { "1+1)*(2+2", 8 },
    { "sin((2)", 0.909297426825 }
#else
    { "(1+1)*(2+2)", 8 },
    { "sin((2))", 0.909297426825 }
#endif
};

static ErrorTest errorTests[NUM_ERROR_TESTS] = {
    { "", PERR_EMPTY },
    { "()", PERR_EMPTY },
    { "x+", PERR_MISSING_OPERAND },
    { "sin(x, y)", PERR_FUNCTION_WRONG_ARITY },
    { "sin,", PERR_UNEXPECTED_DELIMITER },
#if PAD_PARENTHESES
    { "(x", PERR_SUCCESS },
    { "x)", PERR_SUCCESS }
#else
    { "(x", PERR_EXCESS_OPENING_PARENTHESIS },
    { "x)", PERR_EXCESS_CLOSING_PARENTHESIS }
#endif
};

bool almost_equals(double a, double b)
{
    return (fabs(a - b) < EPSILON);
}

int perform_value_tests(ParsingContext *ctx)
{
    Node *node = NULL;
    
    for (int i = 0; i < NUM_VALUE_TESTS; i++)
    {
        if (parse_input(ctx, valueTests[i].input, PAD_PARENTHESES, &node) != PERR_SUCCESS
            || !almost_equals(arith_eval(node), valueTests[i].result))
        {
            return i;
        }
    }
    
    return -1;
}

int perform_error_tests(ParsingContext *ctx)
{
    Node *node = NULL;

    for (int i = 0; i < NUM_ERROR_TESTS; i++)
    {
        if (parse_input(ctx, errorTests[i].input, PAD_PARENTHESES, &node) != errorTests[i].result)
        {
            return i;
        }
    }

    return -1;
}

int main()
{
    ParsingContext *ctx = arith_get_ctx();
    int error_index = perform_value_tests(ctx);
    
    if (error_index != -1)
    {
        printf(F_RED "\nError in value test %d: '%s'" COL_RESET "\n", error_index, valueTests[error_index].input);
        return 1;
    }

    error_index = perform_error_tests(ctx);

    if (error_index != -1)
    {
        printf(F_RED "\nError in error test %d: '%s'" COL_RESET "\n", error_index, errorTests[error_index].input);
        return 1;
    }

    printf(F_GREEN "passed (PAD_P: %s, Version: %s, Tests: %d)" COL_RESET "\n",
        PAD_PARENTHESES ? "1" : "0",
        VERSION,
        NUM_VALUE_TESTS + NUM_ERROR_TESTS);

    return 0;
}
