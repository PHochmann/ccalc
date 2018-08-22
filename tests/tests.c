#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "../src/engine/constants.h"
#include "../src/engine/context.h"
#include "../src/engine/node.h"
#include "../src/engine/parser.h"

#include "../src/arith.h"

#define EPSILON 0.000001
#define NUM_TESTS 17

char *inputs[] = {
	"1",
	"1.1",
	" ( 9.0 *  0)",
	"1+2",
	"2 2",
	"-1",
	"1%",
	"1+2*3+4",
	"sin(0)",
	"sin0+2",
	"sum()",
	"1+sum()*2",
	"sum(1, 2, 3)*2",
	"pi",
	"-pi e",
	"((--1)) sum2 !%",
	"--(1+sum(ld--8, --1%+--1%, 2 2))%+1",
};

double results[] = {
	1,
	1.1,
	0,
	3,
	4,
	-1,
	0.01,
	11,
	0,
	2,
	0,
	1,
	12,
	3.14159265359,
	-8.5397342226,
	0.02,
	1.0802,
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
		if (parse_node(&context, inputs[i], &node) != PERR_SUCCESS)
		{
			return i;
		}
		
		if (!almost_equals(arith_eval(node), results[i]))
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
		return 0;
	}
	else
	{
		printf(F_RED "Error in test case %d: '%s'" COL_RESET "\n", error_index, inputs[error_index]);
		return 1;
	}
}
