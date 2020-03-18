#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <float.h>
#include <math.h>

#include "evaluation.h"
#include "arith_context.h"

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

bool op_eval(Operator *op, size_t num_args, double *args, double *out)
{
    switch ((size_t)(op - g_ctx->operators))
    {
        case 2: // x+y
            *out = args[0] + args[1];
            return true;
        case 3: // x-y
            *out = args[0] - args[1];
            return true;
        case 4: // x*y
            *out = args[0] * args[1];
            return true;
        case 5: // x/y
            *out = args[0] / args[1];
            return true;
        case 6: // x^y
            *out = pow(args[0], args[1]);
            return true;
        case 7: // x C y
            *out = binomial(args[0], args[1]);
            return true;
        case 8: // x mod y
            *out = fmod(args[0], args[1]);
            return true;
        case 9: // -x
            *out = -args[0];
            return true;
        case 10: // +x
            *out = args[0];
            return true;
        case 11: // x!
        {
            double res = 1;
            for (double i = trunc(args[0]); i > 1; i--)
            {
                res *= i;
            }
            *out = res;
            return true;
        }
        case 12: // x%
            *out = args[0] / 100;
            return true;
        case 13: // exp(x)
            *out = exp(args[0]);
            return true;
        case 14: // root(x, n)
            *out = pow(args[0], 1 / args[1]);
            return true;
        case 15: // sqrt(x)
            *out = sqrt(args[0]);
            return true;
        case 16: // log(x, n)
            *out = log(args[0]) / log(args[1]);
            return true;
        case 17: // ln(x)
            *out = log(args[0]);
            return true;
        case 18: // ld(x)
            *out = log2(args[0]);
            return true;
        case 19: // log(x)
            *out = log10(args[0]);
            return true;
        case 20: // sin(x)
            *out = sin(args[0]);
            return true;
        case 21: // cos(x)
            *out = cos(args[0]);
            return true;
        case 22: // tan(x)
            *out = tan(args[0]);
            return true;
        case 23: // asin(x)
            *out = asin(args[0]);
            return true;
        case 24: // acos(x)
            *out = acos(args[0]);
            return true;
        case 25: // atan(x)
            *out = atan(args[0]);
            return true;
        case 26: // sinh(x)
            *out = sinh(args[0]);
            return true;
        case 27: // cosh(x)
            *out = cosh(args[0]);
            return true;
        case 28: // tanh(x)
            *out = tanh(args[0]);
            return true;
        case 29: // asinh(x)
            *out = asinh(args[0]);
            return true;
        case 30: // acosh(x)
            *out = acosh(args[0]);
            return true;
        case 31: // atanh(x)
            *out = atanh(args[0]);
            return true;
        case 32: // max(x, y, ...)
        {
            double res = -INFINITY;
            for (size_t i = 0; i < num_args; i++)
            {
                double child_val = args[i];
                if (child_val > res) res = child_val;
            }
            *out = res;
            return true;
        }
        case 33: // min(x, y, ...)
        {
            double res = INFINITY;
            for (size_t i = 0; i < num_args; i++)
            {
                double child_val = args[i];
                if (child_val < res) res = child_val;
            }
            *out = res;
            return true;
        }
        case 34: // abs(x)
            *out = fabs(args[0]);
            return true;
        case 35: // ceil(x)
            *out = ceil(args[0]);
            return true;
        case 36: // floor(x)
            *out = floor(args[0]);
            return true;
        case 37: // round(x)
            *out = round(args[0]);
            return true;
        case 38: // trunc(x)
            *out = trunc(args[0]);
            return true;
        case 39: // frac(x)
            *out = args[0] - floor(args[0]);
            return true;
        case 40: // sgn(x)
            *out = args[0] < 0 ? -1 : (args[0] > 0) ? 1 : 0;
            return true;
        case 41: // sum(x, y, ...)
        {
            double res = 0;
            for (size_t i = 0; i < num_args; i++) res += args[i];
            *out = res;
            return true;
        }
        case 42: // prod(x, y, ...)
        {
            double res = 1;
            for (size_t i = 0; i < num_args; i++) res *= args[i];
            *out = res;
            return true;
        }
        case 43: // avg(x, y, ...)
        {
            if (num_args == 0) *out = 0;
            double res = 0;
            for (size_t i = 0; i < num_args; i++) res += args[i];
            *out = res / num_args;
            return true;
        }
        case 44: // gcd(x, y)
            *out = euclid(args[0], args[1]);
            return true;
        case 45: // lcm(x, y)
            *out = fabs(trunc(args[0]) * trunc(args[1])) / euclid(args[0], args[1]);
            return true;
        case 46: // rand(x, y)
            *out = random_between(args[0], args[1]);
            return true;
        case 47: // fib(x)
            *out = fibonacci(args[0]);
            return true;
        case 48: // gamma(x)
            *out = tgamma(args[0]);
            return true;
        case 49: // pi
            *out = 3.14159265359;
            return true;
        case 50: // e
            *out = 2.71828182846;
            return true;
        case 51: // phi
            *out = 1.61803398874;
            return true;
        case 52: // clight (m/s)
            *out = 299792458;
            return true;
        case 53: // csound (m/s)
            *out = 343.2;
            return true;
    }

    printf("Error: No reduction possible for operator %s.\n", op->name);
    return false;
}

double arith_evaluate(Node *tree)
{
    return convenient_reduce(tree, op_eval);
}