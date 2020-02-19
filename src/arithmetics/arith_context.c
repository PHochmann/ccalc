#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <float.h>
#include <math.h>

#include "arith_context.h"

ParsingContext __g_ctx;
Operator operators[ARITH_MAX_OPS];

double euclid(double a, double b)
{
    a = fabs(trunc(a));
    b = fabs(trunc(b));

    if (a == 0)
    {
        return b;
    }
    else
    {
        while (b != 0)
        {
            if (a > b)
            {
                a = a - b;
            }
            else
            {
                b = b - a;
            }
        }
        return a;
    }
}

double binomial(double n, double k)
{
    n = fabs(trunc(n));
    k = fabs(trunc(k));

    if (k == 0) return 1;
    if ((2 * k) > n) k = n - k;
    
    double res = 1;
    for (double i = 1; i <= k; i++)
    {
        res = (res * (n - k + i)) / i;
    }
    
    return res;
}

long fib(long n)
{
    if (n == 0) return 0;
    if (n == 1) return 1;
    if (n == -1) return 1;
    return fib(n - 1) + fib(n - 2); 
}

double fibonacci(double n)
{
    long l = (long)n;

    if (l < 0) // Generalization to negative numbers
    {
        if (l % 2 == 0)
        {
            return -fib(labs(l));
        }
        else
        {
            return fib(labs(l));
        }
    }
    else
    {
        return fib(l);
    }
}

/*
Returns: Random natural number between min and max - 1 (i.e. max is exclusive)
*/
double random_between(double min, double max)
{
    min = trunc(min);
    max = trunc(max);
    long diff = (long)(max - min);
    if (diff < 1) return -1;
    return rand() % diff + min;
}

/*
Summary: Evaluates operator tree
Returns: Result after recursive application of all operators
*/
double arith_eval(Node *tree)
{
    switch (get_type(tree))
    {
        case NTYPE_CONSTANT:
        {
            return get_const_value(tree);
        }
        case NTYPE_OPERATOR:
        {
            size_t num_args = get_num_children(tree);
            double args[num_args];
            for (size_t i = 0; i < num_args; i++)
            {
                args[i] = arith_eval(get_child(tree, i));
            }
            return op_eval(get_op(tree), num_args, args);
        }
        case NTYPE_VARIABLE:
        {
            printf("Error: Encountered variable in arith_eval.\n");
            return -1;
        }
    }

    return 0;
}

double op_eval(Operator *op, size_t num_args, double *args)
{
    switch ((size_t)(op - operators))
    {
        case 0: // $x
            printf("Warning: Tried to evaluate $ operator in op_eval.\n");
            return args[0];
        case 1: // x@y
            return 0;
        case 2: // x+y
            return args[0] + args[1];
        case 3: // x-y
            return args[0] - args[1];
        case 4: // x*y
            return args[0] * args[1];
        case 5: // x/y
            return args[0] / args[1];
        case 6: // x^y
            return pow(args[0], args[1]);
        case 7: // x C y
            return binomial(args[0], args[1]);
        case 8: // x mod y
            return fmod(args[0], args[1]);
        case 9: // -x
            return -args[0];
        case 10: // +x
            return args[0];
        case 11: // x!
        {
            double res = 1;
            for (double i = trunc(args[0]); i > 1; i--)
            {
                res *= i;
            }
            return res;
        }
        case 12: // x%
            return args[0] / 100;
        case 13: // exp(x)
            return exp(args[0]);
        case 14: // root(x, n)
            return pow(args[0], 1 / args[1]);
        case 15: // sqrt(x)
            return sqrt(args[0]);
        case 16: // log(x, n)
            return log(args[0]) / log(args[1]);
        case 17: // ln(x)
            return log(args[0]);
        case 18: // ld(x)
            return log2(args[0]);
        case 19: // log(x)
            return log10(args[0]);
        case 20: // sin(x)
            return sin(args[0]);
        case 21: // cos(x)
            return cos(args[0]);
        case 22: // tan(x)
            return tan(args[0]);
        case 23: // asin(x)
            return asin(args[0]);
        case 24: // acos(x)
            return acos(args[0]);
        case 25: // atan(x)
            return atan(args[0]);
        case 26: // sinh(x)
            return sinh(args[0]);
        case 27: // cosh(x)
            return cosh(args[0]);
        case 28: // tanh(x)
            return tanh(args[0]);
        case 29: // asinh(x)
            return asinh(args[0]);
        case 30: // acosh(x)
            return acosh(args[0]);
        case 31: // atanh(x)
            return atanh(args[0]);
        case 32: // max(x, y, ...)
        {
            double res = -INFINITY;
            for (size_t i = 0; i < num_args; i++)
            {
                double child_val = args[i];
                if (child_val > res) res = child_val;
            }
            return res;
        }
        case 33: // min(x, y, ...)
        {
            double res = INFINITY;
            for (size_t i = 0; i < num_args; i++)
            {
                double child_val = args[i];
                if (child_val < res) res = child_val;
            }
            return res;
        }
        case 34: // abs(x)
            return fabs(args[0]);
        case 35: // ceil(x)
            return ceil(args[0]);
        case 36: // floor(x)
            return floor(args[0]);
        case 37: // round(x)
            return round(args[0]);
        case 38: // trunc(x)
            return trunc(args[0]);
        case 39: // frac(x)
            return args[0] - floor(args[0]);
        case 40: // sgn(x)
            return args[0] < 0 ? -1 : (args[0] > 0) ? 1 : 0;
        case 41: // sum(x, y, ...)
        {
            double res = 0;
            for (size_t i = 0; i < num_args; i++) res += args[i];
            return res;
        }
        case 42: // prod(x, y, ...)
        {
            double res = 1;
            for (size_t i = 0; i < num_args; i++) res *= args[i];
            return res;
        }
        case 43: // avg(x, y, ...)
        {
            if (num_args == 0) return 0;
            double res = 0;
            for (size_t i = 0; i < num_args; i++) res += args[i];
            return res / num_args;
        }
        case 44: // gcd(x, y)
            return euclid(args[0], args[1]);
        case 45: // lcm(x, y)
            return fabs(trunc(args[0]) * trunc(args[1])) / euclid(args[0], args[1]);
        case 46: // rand(x, y)
            return random_between(args[0], args[1]);
        case 47: // fib(x)
            return fibonacci(args[0]);
        case 48: // gamma(x)
            return tgamma(args[0]);
        case 49: // pi
            return 3.14159265359;
        case 50: // e
            return 2.71828182846;
        case 51: // phi
            return 1.61803398874;
        case 52: // clight (m/s)
            return 299792458;
        case 53: // csound (m/s)
            return 343.2;
    }

    printf("Error: No evaluation case in op_eval.\n");
    return -1;
}

/*
Summary: Used to delete user-defined function operators
*/
void arith_reset_ctx()
{
    // As operator names are stored on heap, we need to free them
    for (size_t i = ARITH_NUM_OPS; i < __g_ctx.num_ops; i++)
    {
        free(__g_ctx.operators[i].name);
    }
    __g_ctx.num_ops = ARITH_NUM_OPS;
}

/*
Summary: Sets arithmetic context stored in global variable
*/
void arith_init_ctx()
{
    __g_ctx = get_context(ARITH_MAX_OPS, operators);
    ctx_add_ops(g_ctx, ARITH_NUM_OPS,
        op_get_prefix("$", 0),
        op_get_prefix("@", 7),
        
        op_get_infix("+", 2, OP_ASSOC_LEFT),
        op_get_infix("-", 2, OP_ASSOC_LEFT),
        op_get_infix("*", 3, OP_ASSOC_LEFT),
        op_get_infix("/", 3, OP_ASSOC_LEFT),
        op_get_infix("^", 4, OP_ASSOC_RIGHT),
        op_get_infix("C", 1, OP_ASSOC_LEFT),
        op_get_infix("mod", 1, OP_ASSOC_LEFT),
        op_get_prefix("-", 6),
        op_get_prefix("+", 6),
        op_get_postfix("!", 5),
        op_get_postfix("%", 5),
        op_get_function("exp", 1),
        op_get_function("root", 2),
        op_get_function("sqrt", 1),
        op_get_function("log", 2),
        op_get_function("ln", 1),
        op_get_function("ld", 1),
        op_get_function("lg", 1),

        op_get_function("sin", 1),
        op_get_function("cos", 1),
        op_get_function("tan", 1),
        op_get_function("asin", 1),
        op_get_function("acos", 1),
        op_get_function("atan", 1),
        op_get_function("sinh", 1),
        op_get_function("cosh", 1),
        op_get_function("tanh", 1),
        op_get_function("asinh", 1),
        op_get_function("acosh", 1),
        op_get_function("atanh", 1),

        op_get_function("max", OP_DYNAMIC_ARITY),
        op_get_function("min", OP_DYNAMIC_ARITY),
        op_get_function("abs", 1),
        op_get_function("ceil", 1),
        op_get_function("floor", 1),
        op_get_function("round", 1),
        op_get_function("trunc", 1),
        op_get_function("frac", 1),
        op_get_function("sgn", 1),

        op_get_function("sum", OP_DYNAMIC_ARITY),
        op_get_function("prod", OP_DYNAMIC_ARITY),
        op_get_function("avg", OP_DYNAMIC_ARITY),
        op_get_function("gcd", 2),
        op_get_function("lcm", 2),
        op_get_function("rand", 2),
        op_get_function("fib", 1),
        op_get_function("gamma", 1),

        op_get_constant("pi"),
        op_get_constant("e"),
        op_get_constant("phi"),
        op_get_constant("clight"),
        op_get_constant("csound"));
    
    // Set multiplication as glue-op
    ctx_set_glue_op(g_ctx, ctx_lookup_op(g_ctx, "*", OP_PLACE_INFIX));
    // Initialize random for rand(2) command
    srand(time(NULL));
}
