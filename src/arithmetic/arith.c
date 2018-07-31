#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "arith.h"

#define ARITH_STRING_LENGTH 30
#define ARITH_NUM_OPS (37+10)

ParsingContext arith_ctx;

long binomial(long n, long k)
{
	if (k == 0) return 1;
	if ((2 * k) > n) k = n - k;
	
	long res = 1;
	for (long i = 1; i <= k; i++)
	{
		res = (res * (n - k + i)) / i;
	}
	
	return res;
}

double arith_eval(Node *node)
{
	double d_res = 1;
	long i_res = 1;
	
	switch (node->type)
	{
		case NTYPE_CONSTANT:
			return *(double*)(node->const_value);
			
		case NTYPE_OPERATOR:
			switch ((size_t)(node->op - arith_ctx.operators))
			{
				case 0:
					return arith_eval(node->children[0]) * arith_eval(node->children[1]);
				case 1:
					return arith_eval(node->children[0]) / arith_eval(node->children[1]);
				case 2:
					return arith_eval(node->children[0]) + arith_eval(node->children[1]);
				case 3:
					return arith_eval(node->children[0]) - arith_eval(node->children[1]);
				case 4:
					return pow(arith_eval(node->children[0]), arith_eval(node->children[1]));
				case 5:
					return -arith_eval(node->children[0]);
				case 6:
					return arith_eval(node->children[0]);
					
				case 7:
					i_res = 1;
					for (long i = labs((long)trunc(arith_eval(node->children[0]))); i > 1; i--) i_res *= i;
					return (double)i_res;
					
				case 8:
					return arith_eval(node->children[0]) / 100;
				case 9:
					return exp(arith_eval(node->children[0]));
				case 10:
					return sqrt(arith_eval(node->children[0]));
				case 11:
					return pow(arith_eval(node->children[0]), 1 / arith_eval(node->children[1]));
				case 12:
					return log(arith_eval(node->children[0])) / log(arith_eval(node->children[1]));
				case 13:
					return log(arith_eval(node->children[0]));
				case 14:
					return log10(arith_eval(node->children[0]));
				case 15:
					return sin(arith_eval(node->children[0]));
				case 16:
					return cos(arith_eval(node->children[0]));
				case 17:
					return tan(arith_eval(node->children[0]));
				case 18:
					return asin(arith_eval(node->children[0]));
				case 19:
					return acos(arith_eval(node->children[0]));
				case 20:
					return atan(arith_eval(node->children[0]));
					
				case 21:
					d_res = -INFINITY;
					for (int i = 0; i < node->num_children; i++)
					{
						double child_val = arith_eval(node->children[i]);
						if (child_val > d_res) d_res = child_val;
					}
					return d_res;
					
				case 22:
					d_res = INFINITY;
					for (int i = 0; i < node->num_children; i++)
					{
						double child_val = arith_eval(node->children[i]);
						if (child_val < d_res) d_res = child_val;
					}
					return d_res;
					
				case 23:
					return fabs(arith_eval(node->children[0]));
				case 24:
					return round(arith_eval(node->children[0]));
				case 25:
					return trunc(arith_eval(node->children[0]));
				case 26:
					return ceil(arith_eval(node->children[0]));
				case 27:
					return floor(arith_eval(node->children[0]));
					
				case 28:
					d_res = 0;
					for (int i = 0; i < node->num_children; i++) d_res += arith_eval(node->children[i]);
					return d_res;
					
				case 29:
					d_res = 1;
					for (int i = 0; i < node->num_children; i++) d_res *= arith_eval(node->children[i]);
					return d_res;
					
				case 30:
					d_res = 0;
					for (int i = 0; i < node->num_children; i++) d_res += arith_eval(node->children[i]);
					return d_res / node->num_children;
				
				case 31:
					return (double)binomial(
						labs((long)trunc(arith_eval(node->children[0]))),
						labs((long)trunc(arith_eval(node->children[1]))));
						
				case 32:
					return fmod(arith_eval(node->children[0]), arith_eval(node->children[1]));
				case 33:
					return tgamma(arith_eval(node->children[0]));
				
				case 34:
					return 3.14159265359;
				case 35:
					return 2.71828182846;
				case 36:
					return 1.61803398874;
					
				default:
					return 0;
			}
			
		default:
			return 0;
	}
}

bool _arith_try_parse(char *in, void *out)
{
	char *ptr;
	*((double*)out) = strtod(in, &ptr);
	return ptr != in; // Unsafe, fix later
}

void _arith_to_string(void *in, char *str, size_t buff_size)
{
	if (buff_size < (ARITH_STRING_LENGTH + 1)) return;
	sprintf(str, "%.30g", *((double*)in));
}

ParsingContext arith_get_ctx()
{
	arith_ctx = get_context(sizeof(double), ARITH_STRING_LENGTH + 1, ARITH_NUM_OPS, _arith_try_parse, _arith_to_string);
	
	add_op(&arith_ctx, op_get_infix("*", 2, OP_ASSOC_BOTH));
	add_op(&arith_ctx, op_get_infix("/", 2, OP_ASSOC_LEFT));
	add_op(&arith_ctx, op_get_infix("+", 1, OP_ASSOC_BOTH));
	add_op(&arith_ctx, op_get_infix("-", 1, OP_ASSOC_LEFT));
	add_op(&arith_ctx, op_get_infix("^", 3, OP_ASSOC_RIGHT));
	
	add_op(&arith_ctx, op_get_prefix("-", 5));
	add_op(&arith_ctx, op_get_prefix("+", 5));
	
	add_op(&arith_ctx, op_get_postfix("!", 4));
	add_op(&arith_ctx, op_get_postfix("%", 4));
	
	add_op(&arith_ctx, op_get_function("exp", 1));
	add_op(&arith_ctx, op_get_function("sqrt", 1));
	add_op(&arith_ctx, op_get_function("root", 2));
	add_op(&arith_ctx, op_get_function("log", 2));
	add_op(&arith_ctx, op_get_function("ln", 1));
	add_op(&arith_ctx, op_get_function("lg", 1));
	add_op(&arith_ctx, op_get_function("sin", 1));
	add_op(&arith_ctx, op_get_function("cos", 1));
	add_op(&arith_ctx, op_get_function("tan", 1));
	add_op(&arith_ctx, op_get_function("asin", 1));
	add_op(&arith_ctx, op_get_function("acos", 1));
	add_op(&arith_ctx, op_get_function("atan", 1));
	add_op(&arith_ctx, op_get_function("max", DYNAMIC_ARITY));
	add_op(&arith_ctx, op_get_function("min", DYNAMIC_ARITY));
	add_op(&arith_ctx, op_get_function("abs", 1));
	add_op(&arith_ctx, op_get_function("round", 1));
	add_op(&arith_ctx, op_get_function("trunc", 1));
	add_op(&arith_ctx, op_get_function("ceil", 1));
	add_op(&arith_ctx, op_get_function("floor", 1));
	add_op(&arith_ctx, op_get_function("sum", DYNAMIC_ARITY));
	add_op(&arith_ctx, op_get_function("prod", DYNAMIC_ARITY));
	add_op(&arith_ctx, op_get_function("avg", DYNAMIC_ARITY));
	
	add_op(&arith_ctx, op_get_infix("C", 1, OP_ASSOC_LEFT));
	add_op(&arith_ctx, op_get_infix("mod", 1, OP_ASSOC_LEFT));
	
	add_op(&arith_ctx, op_get_function("gamma", 1));
	
	add_op(&arith_ctx, op_get_constant("pi"));
	add_op(&arith_ctx, op_get_constant("e"));
	add_op(&arith_ctx, op_get_constant("phi"));
	
	set_glue_op(&arith_ctx, &arith_ctx.operators[0]);
	
	return arith_ctx;
}
