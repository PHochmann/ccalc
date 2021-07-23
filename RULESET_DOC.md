# Ruleset documentation

*Todo: expand and describe semantics more formally*

```ccalc``` contains an interpreter for rulesets to perform algebraic simplifications.
A rule has the form of ```before -> after WHERE restrictionA ; restrictionB...``` with the number of restrictions being
limited by ```MATCHING_MAX_CONSTRAINTS``` in ```matching.h```.
Within a rule, a variable contained in brackets can be bound to more than one operand, which is useful for
functions with no arity restriction like ```sum, prod, avg```.

## Example 1

The following rule filters zeros from sums: ```sum([xs], 0, [ys]) -> sum([xs], [ys])```.
Here, ```[xs]``` and ```[ys]``` are arbitrarily many operands coming before and after the zero.
The rule does not contain any restrictions.

## Example 2

The following rule deletes minuses when exponentiating with an even exponent: ```(-x)^y -> x^y WHERE (y mod 2) == 0```.
It contains a single restriction.

## Additional operators

The following operators that can be used in *before*, *after* and restrictions, additionally to the arithmetical operators:
``` type(x), count(*), equal(x, y), ==, !=, >, <, >=, <=, ||, ! TRUE, FALSE, CONST, VAR, OP ```
(see ```propositional_context.c``` and ```propositional_evaluation.c``` for details).

## Restrictions (WHERE-clauses)

A restriction limits the cases a rule can be applied to.
```;``` is treated as a logical *and*, thus all restrictions must be fulfilled for the rule to be executed.
A restriction is evaluated as soon as all the variable it contains are bound by the matching algorithm to falsify
a restriction as soon as possible.
This is why there is no *and*-operator to use within a restriction.
It is better to write multiple restrictions containing less variables each than to write one big conjunctive restriction
containing all the variables that could only be checked after binding all the variables.

A constant tree is a tree that only consists of a single, constant number.
```type, count, equal``` can be applied to any tree and evaluate to a constant.
All other operators evaluate to ```FALSE``` when not applied to a constant tree.

A restriction holds if it does not evaluate to ```FALSE```.
Thus, in example 1, ```(y mod 2) == 0``` will not hold if ```y``` is not a constant.
Since the tree representing, for example, the expression ```1+1``` is not constant,
it is recommended to reduce subtrees that do not contain a variable to their numerical approximation.
This introduces inaccuracies, making ```ccalc```'s simplification algorithm not purely algebraical.
