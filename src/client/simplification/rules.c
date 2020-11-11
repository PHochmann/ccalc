#include "rules.h"

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
    "0+x               -> x\n"
    "1*x               -> x\n"
    "x*1               -> x\n" // Smart but redundant elimination rules for performance
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
    "deriv(x^y, z)     -> (y * deriv(x, z) * x^-1 + deriv(y, z) * ln(x)) * x^y\n"
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

    "(-x)^y -> x^y       WHERE    type(y) = CONST ; y mod 2 = 0\n"

    // Flatten again and easy simplification
    "prod()                       -> 1\n"
    "prod(x)                      -> x\n"
    "sum()                        -> 0\n"
    "sum(x)                       -> x\n"
    "prod([_], 0, [_])            -> 0\n"
    "prod([xs], 1, [ys])          -> prod([xs], [ys])\n"
    "sum([xs], 0, [ys])           -> sum([xs], [ys])\n"
    "sum([xs], sum([ys]), [zs])   -> sum([xs], [ys], [zs])\n"
    "prod([xs], prod([ys]), [zs]) -> prod([xs], [ys], [zs])\n"

    // Misc.
    "--x -> x\n"

    // Operator ordering
    "sum([xs], x^y, [ys], prod([yys]), [zs]) -> sum([xs], prod([yys]), [ys], x^y, [zs])\n" // Products before powers
    "prod([xs], x^y, [ys], sum([yys]), [zs]) -> prod([xs], sum([yys]), [ys], x^y, [zs])\n" // Sums before powers

    // Multiplication of constants
    "prod(cX, [xs], sum(cY, [xxs]), [zs])    -> prod([xs], sum(cX*cY, prod(cX, sum([xxs]))), [zs])\n"
    // Agressive multiplication
    //"prod(x, [xs], sum(y, [xxs]), [zs])    -> prod([xs], sum(prod(x,y), prod(x, sum([xxs]))), [zs])\n"

    // Simplify sums
    "sum([xs], x, [ys], -x, [zs]) -> sum([xs], [ys], [zs])\n"
    "sum([xs], -x, [ys], x, [zs]) -> sum([xs], [ys], [zs])\n"
    "-sum(x,[xs])                 -> sum(-x,-sum([xs]))\n" // Pull minus into sum

    // No minuses in or before products
    "-prod([xs])                      -> prod(-1, [xs])\n"
    "prod([xs],-x,[zs])               -> prod(-1, [xs], x, [zs])\n"
    "prod(cX,[xs],-x,[ys])             -> prod(-cX,[xs],x,[ys])\n"

    // Products within sum
    "sum([xs], x, [ys], x, [zs]) -> sum([xs], prod(2,x), [ys], [zs])\n"

    "sum([xs], prod([xxs], x, [yys]), [ys], -x, [zs]) -> sum([xs], prod(sum(-1, prod([xxs], [yys])), x), [ys], [zs])\n"
    "sum([xs], -x, [ys], prod([xxs], x, [yys]), [zs]) -> sum([xs], prod(sum(-1, prod([xxs], [yys])), x), [ys], [zs])\n"
    "sum([xs], prod([xxs], dX, [yys]), [ys], dX, [zs]) -> sum([xs], prod(sum(1, prod([xxs], [yys])), dX), [ys], [zs])\n"
    "sum([xs], dX, [ys], prod([xxs], dX, [yys]), [zs]) -> sum([xs], prod(sum(1, prod([xxs], [yys])), dX), [ys], [zs])\n"

    "sum([xs], prod([xxs], x, [yys]), [ys], prod([xxxs], x, [yyys]), [zs])"
        "-> sum([xs], prod(sum(prod([xxs], [yys]), prod([xxxs], [yyys])), x), [ys], [zs])\n"

    // Sums within products

    // The following three rules pull a value of a common base into a product
    // e.g.: x + a*x^z -> (a+z^(1-z))*x^z
    /*"sum([xs], prod([xxs], x^z, [yys]), [ys], x^y, [zs]) ->"
        " sum([xs], prod(sum(prod([xxs], [yys]), x^sum(y, -z)), x^z), [ys], [zs])\n"

    "sum([xs], prod([xxs], x^z, [yys]), [ys], x, [zs]) ->"
        " sum([xs], prod(sum(prod([xxs], [yys]), x^sum(1, -z)), x^z), [ys], [zs])\n"

    "sum([xs], x, [ys], prod([xxs], x^z, [yys]), [zs]) ->"
        " sum([xs], prod(sum(prod([xxs], [yys]), x^sum(1, -z)), x^z), [ys], [zs])\n"

    "sum([xs], prod([xxs], x, [yys]), [ys], x^y, [zs]) ->"
        " sum([xs], prod(sum(prod([xxs], [yys]), x^sum(y, -1)), x), [ys], [zs])\n"*/

    // Binomische Formel rückwärts
    "sum(prod(2, x, y), x^2, y^2) -> sum(x, y)^2\n"
    "sum(prod(2, y, x), x^2, y^2) -> sum(x, y)^2\n"

    // Powers
    "x^1                              -> x\n"
    "(x^y)^z                          -> x^prod(y, z)\n"
    "prod([xs], x^y, [ys], x^z, [zs]) -> prod([xs], x^sum(y,z), [ys], [zs])\n"
    "prod([xs], dY^x, [ys], dZ^x, [zs]) -> prod([xs], prod(dY, dZ)^x, [ys], [zs])\n"
    "x^0                              -> 1\n"
    "prod([xs], x, [ys], x, [zs])     -> prod([xs], x^2, [ys], [zs])\n"
    "prod([xs], x, [ys], x^y, [zs])   -> prod([xs], x^sum(y,1), [ys], [zs])\n"
    "prod(x, y)^z                     -> prod(x^z, y^z)\n"
    "prod([xs], x^y, [ys], x, [zs])   -> prod([xs], x^sum(y,1), [ys], [zs])\n"
    "prod([xs], x^z, [ys], x^y, [zs]) -> prod([xs], x^sum(y,z), [ys], [zs])\n"

    "prod([xs], sum([xxs], x^y, [yys]), [ys], x^z, [zs]) -> sum(prod([xs], sum([xxs], [yys]), [ys], x^z, [zs]), x^sum(y,z))\n"

    // Trigonometrics
    "sum([xs], cos(x)^2, [ys], sin(x)^2, [zs])   -> sum(1, [xs], [ys], [zs])\n"
    "sum([xs], sin(x)^2, [ys], cos(x)^2, [zs])   -> sum(1, [xs], [ys], [zs])\n"
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

// Fold flattened operators again
    "sum(x)                                  -> x\n"
    "prod(x)                                 -> x\n"

    "prod(-x, [xs])                          -> -prod(x, [xs])\n"
    "sum([xs],-prod([xxs]),[ys])             -> sum([xs],[ys])-prod([xxs])\n"
    "-sum([xs], x)                           -> -sum([xs])-x\n"
    "sum([xs], x)                            -> sum([xs])+x\n"
    "prod([xs], x)                           -> prod([xs])*x\n"

    "sum([xs], prod([xxs], -x, [yys]), [ys]) -> sum([xs], [ys]) - prod([xxs],x,[yys])\n"
    "prod([xs], x)^cY                        -> prod([xs])^cY * x^cY\n"
    "prod()                                  -> 1\n"
    "sum()                                   -> 0",

// Make output pretty (sugarfy)
    "0*x           -> 0\n"
    "1*x           -> x\n"
    "0+x           -> x\n"
    "x^(-y)        -> 1/x^y\n"
    "x^0.5         -> sqrt(x)\n"
    "x^1           -> x\n"
    "-(x+y)        -> x-y\n"
    "(-1)*x        -> -x\n"

    "x*(cY/z)      -> (cY*x)/z\n"
    "x+((-y)/z)    -> x-(y/z)\n"
    "x+(-y*z)      -> x-y*z\n"
    "cX*(x+y)      -> cX*x + cX*y\n"
    "--x           -> x\n"
    "(-1)*x        -> -x\n"
    "x+(-y)        -> x-y\n"
    "(-x)+y        -> y-x\n"
    "-(x*y)        -> (-x)*y\n"
    "root(x, 2)    -> sqrt(x)\n"
    "x^(1/y)       -> root(x, y)\n"
    
    "x+(y+z)       -> x+y+z\n"
    "x*(y*z)       -> x*y*z",

// Ordering
    "dX*cX         -> cX*dX\n" // Constants before
    "dX+cX         -> cX+dX\n"
    "dX*-cX        -> -cX*dX\n"
    "dX+cX         -> cX+dX"
};
