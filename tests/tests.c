#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "../src/engine/constants.h"
#include "../src/engine/context.h"
#include "../src/engine/node.h"
#include "../src/engine/parser.h"

#include "../src/arith.h"

#define EPSILON 0.0000001
#define NUM_TESTS 27

typedef struct {
    char *input;
    double result; 
} TestCase;

static TestCase cases[NUM_TESTS] = {
    { "1", 1 },
    { "1.1", 1.1 },
    { "1+2", 3 },
    { "2 2", 4 },
    { "-1", -1 },
    { "1%", 0.01 },
    { "1+2*3+4", 11 },
    { " ( 9.0 *  0)", 0 },
    { "sin(0)", 0 },
    { "sin0+2", 2 },
    { "(cos sin.123pi.123)", 0.38351121094 },
    { "sum()", 0 },
    { "1+sum()*2", 1 },
    { "sum(1, 2, 3)*2", 12 },
    { "pi", 3.14159265359 },
    { "-pi e", -8.5397342226 },
    { "((--1)) sum2 !%", 0.02 },
    { "--(1+sum(ld--8, --1%+--1%, 2 .2))%+1", 1.0442 },
    { "2^2^3", 256 },
    { "(2^2)^3", 64 },
    { "2^2^3 - 2^(2^3)", 0 },
    { "1/2/2", 0.25 },
    { "1/(2/2)", 1 },
    { "2*3^2", 18 },
    { "sin(asin(.2))", 0.2 },
    { "count(1, 2)", 2 },
    { "sqrt(abs(--2!!*--count(1, 2, 3, 4)*--2!!))", 4 }
};

bool almost_equals(double a, double b)
{
    return (fabs(a - b) < EPSILON);
}

int perform_tests()
{
    ParsingContext context = arith_get_ctx();
    Node *node = NULL;
    
    for (int i = 0; i < NUM_TESTS; i++)
    {
        if (parse_node(&context, cases[i].input, &node) != PERR_SUCCESS)
        {
            return i;
        }
        
        if (!almost_equals(arith_eval(node), cases[i].result))
        {
            return i;
        }
    }
    
    
    return -1;
}

int main(int argc, char *argv[])
{
    init_parser();
    int error_index = perform_tests();
    uninit_parser();
    
    if (error_index == -1)
    {
        printf(F_GREEN "passed" COL_RESET "\n");
        return 0;
    }
    else
    {
        printf(F_RED "\nError in test case %d: '%s'" COL_RESET "\n", error_index, cases[error_index].input);
        return 1;
    }
}
