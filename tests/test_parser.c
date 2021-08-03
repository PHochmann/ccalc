#include <stdio.h>
#include <math.h>

#include "../src/engine/parsing/parser.h"
#include "../src/engine/parsing/context.h"
#include "../src/engine/tree/node.h"
#include "../src/client/core/arith_context.h"
#include "../src/client/core/arith_evaluation.h"
#include "test_parser.h"

// To check if parsed tree evaluates to expected value
struct ValueTest {
    char *input;
    double result; 
};

// To check if parser returns expected error on malformed inputs
struct ErrorTest {
    char *input;
    ParserError result;
};

static const size_t NUM_VALUE_CASES = 43;
static struct ValueTest valueTests[] = {
    // 1. Basic prefix, infix, postfix
    { "2+3",         5 },
    { "2-3",        -1 },
    { "2*3",         6 },
    { "4/2",         2 },
    { "2^3",         8 },
    { "-3",         -3 },
    { "+99",        99 },
    { "4!",         24 },
    { "3%",          0.03 },
    { "1 2 3 4 5", 120 },
    // 2. Correct implementation of evaluation (ToDo: extend)
    { "fib(7)",         13 },
    { "fib(-8)",       -21 },
    { "gcd(942, 492)",   6 },
    { "lcm(14, 24)",   168 },
    // 3. Precedence and parentheses
    { "1+2*3+4",      11 },
    { "1+2*(3+4)",    15 },
    { " ( 9.0 *  2)", 18 },
    // 4. Associativity
    { "1-2-3",               -4 },
    { "1-2-3 - ((1-2)-3)",    0 },
    { "2^2^3",              256 },
    { "2^2^3 - 2^(2^3)",      0 },
    { "(2^2)^3",             64 },
    // 5. Functions
    // 5.1. Constants
    { "pi",        3.141592653 },
    { "pi + 2",    5.141592653 },
    { "3+pi",      6.141592653 },
    { "pi2" ,      6.283185307 },
    { "pi(2)",     6.283185307 },
    { "sum(2)",    2 },
    { "sum()",     0 },
    { "sum() + 2", 2 },
    { "3+sum()",   3 },
    { "prod()2" ,  2 },
    { "prod()(2)", 2 },
    // 5.2. Unary functions
    { "sin(2)",      0.909297426 },
    { "sin(2)*3",    2.727892280 },
    { "sin(-2)%*3", -0.027278922 },
    // 5.3. Binary functions and dynamic arity
    { "log(2 64, 1+1)",              7 },
    { "sum(1,2,3)",                  6 },
    { "prod(2,3,4)-4!",              0 },
    { "sum(sum(1,2),sum(3,4),5)+6", 21 },
    // 6. Going wild
    { "5 .5sin(2)+5pi5", 80.81305990681 },
    { "--(1+sum(ld(--8), --1%+--1%, 2 .2))%+1", 1.0442 },
    { "-sqrt(abs(--2!!*--sum(-1+.2-.2+2, 2^2^3-255, -sum(.1, .9), 1+2)*--2!!))", -4 },
};

static const size_t NUM_ERROR_CASES = 27;
static struct ErrorTest errorTests[] = {
    { "",            PERR_UNEXPECTED_END_OF_EXPR },
    { "     ",       PERR_UNEXPECTED_END_OF_EXPR },
    { "x+",          PERR_UNEXPECTED_END_OF_EXPR },
    { "sin",         PERR_EXPECTED_PARAM_LIST },
    { "sin 2",       PERR_EXPECTED_PARAM_LIST },
    { "sin(x, y)",   PERR_FUNCTION_WRONG_ARITY },
    { "root(x)",     PERR_FUNCTION_WRONG_ARITY },
    { "(1+log(x))",  PERR_FUNCTION_WRONG_ARITY },
    { "2,",          PERR_UNEXPECTED_DELIMITER },
    { ",",           PERR_UNEXPECTED_DELIMITER },
    { "-(1,2)",      PERR_UNEXPECTED_DELIMITER },
    { "sum(3+, 7)",  PERR_UNEXPECTED_DELIMITER },
    { "(x",          PERR_EXCESS_OPENING_PAREN },
    { "(+2",         PERR_EXCESS_OPENING_PAREN },

    { "x)",          PERR_UNEXPECTED_CLOSING_PAREN },
    { "()+2",        PERR_UNEXPECTED_CLOSING_PAREN },
    { "root(x,)",    PERR_UNEXPECTED_CLOSING_PAREN },
    { ")",           PERR_UNEXPECTED_CLOSING_PAREN },
    { "()",          PERR_UNEXPECTED_CLOSING_PAREN },
    { "sin(())",     PERR_UNEXPECTED_CLOSING_PAREN },
    { "sin(2,())",   PERR_UNEXPECTED_CLOSING_PAREN },
    { "sum(+)",      PERR_UNEXPECTED_CLOSING_PAREN },

    { "sum(*)",      PERR_UNEXPECTED_INFIX },
    { "a+(*",        PERR_UNEXPECTED_INFIX },
    { "a b",         PERR_EXPECTED_INFIX },
    { "sin(x)(a+b)", PERR_EXPECTED_INFIX },
    { ".",           PERR_UNEXPECTED_CHARACTER }
};

static const double EPSILON = 0.00000001;
bool almost_equals(double a, double b)
{
    return (fabs(a - b) < EPSILON);
}

bool parser_test(StringBuilder *error_builder)
{
    ParsingContext ctx = get_arith_ctx();

    // Perform value tests
    for (size_t i = 0; i < NUM_VALUE_CASES; i++)
    {
        Node *node = parse_easy(&ctx, valueTests[i].input);
        if (node == NULL)
        {
            ERROR("Parser Error for '%s'\n", valueTests[i].input);
        }

        bool is_equal = almost_equals(arith_evaluate(node), valueTests[i].result);
        free_tree(node);

        if (!is_equal)
        {
            ERROR("Unexpected result for '%s'\n", valueTests[i].input);
        }
    }

    // Perform error tests
    // Remove glue-op to test for "expected infix or prefix"
    ctx_set_glue_op(&ctx, NULL);
    for (size_t i = 0; i < NUM_ERROR_CASES; i++)
    {
        ParsingResult res;
        parse_input(&ctx, errorTests[i].input, &res);
        if (res.error != errorTests[i].result)
        {
            ERROR("Unexpected error type for '%s'\n", errorTests[i].input);
        }
        free_result(&res, true);
    }

    ctx_destroy(&ctx);
    return true;
}

Test get_parser_test()
{
    return (Test){
        parser_test,
        "Parser"
    };
}
