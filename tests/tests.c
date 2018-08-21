#include <stdbool.h>
#include <stdio.h>

#include "../src/engine/constants.h"
#include "../src/engine/context.h"
#include "../src/engine/node.h"
#include "../src/engine/parser.h"

#include "../src/arith.h"

#define EPSILON 0.0001
#define NUM_TESTS 10

char *inputs[] = {
	"1+1",
	"cos(0)+2",
	"sum()",
	"sum(--1--1, 0, 0)+1",
	"sin0",
	"sin0+1",
	"(((2^3^4)))",
	"-cos0%",
	"2*(1+1)*3",
	"2*1+1*3"
};

double results[] = {
	2,
	3,
	0,
	3,
	0,
	1,
	2417851639229258349412352.0,
	-0.01,
	12,
	5,
};

bool almost_equal(double a, double b)
{
	return ((a - b) < EPSILON);
}

bool perform_tests()
{
	init_parser();
	
	ParsingContext context = arith_get_ctx();
	Node *node = NULL;
	
	for (int i = 0; i < NUM_TESTS; i++)
	{
		if (parse_node(&context, inputs[i], &node) != PERR_SUCCESS)
		{
			return false;
		}
		
		if (!almost_equal(arith_eval(node), results[i]))
		{
			printf("%d ", i);
			return false;
		}
	}
	
	uninit_parser();
	
	return true;
}

int main(int argc, char *argv[])
{
	if (perform_tests())
	{
		return 0;
	}
	else
	{
		printf(F_RED "Error in tests!" COL_RESET "\n");
		return 1;
	}
}