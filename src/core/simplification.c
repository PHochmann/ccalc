#include <math.h>

#include "../tree/tree_util.h"
#include "../tree/tree_to_string.h"
#include "../parsing/parser.h"
#include "../transformation/rewrite_rule.h"
#include "simplification.h"
#include "arith_context.h"
#include "evaluation.h"
#include "../util/console_util.h"

#define P(x) parse_conveniently(g_ctx, x)

Node *deriv_before;
Node *deriv_after;
Node *malformed_derivA;
Node *malformed_derivB;

#define NUM_OP_ELIMINATION_RULES 9
RewriteRule op_elimination_rules[NUM_OP_ELIMINATION_RULES];
char *op_elimination_strings[] = {
    "+x", "x",
    "$x", "x",
    "--x", "x",
    "x%", "x/100",
    "x/y", "x*y^(-1)",
    "log(x, e)", "ln(x)",
    "tan(x)", "sin(x)/cos(x)",
    "sqrt(x)", "x^0.5",
    "root(x, y)", "x^(1/y)",
};

#define NUM_DERIVATION_RULES 15
RewriteRule derivation_rules[NUM_DERIVATION_RULES];
char *derivation_rule_strings[] = {
    "deriv(cX*z, z)", "cX",
    "deriv(cX*z^cZ, z)", "cZ*cX*z^(cZ-1)",
    "deriv(-x, z)", "-deriv(x, z)",
    "deriv(x + y, z)", "deriv(x, z) + deriv(y, z)",
    "deriv(x - y, z)", "deriv(x, z) - deriv(y, z)",
    "deriv(x, x)", "1",
    "deriv(bX, z)", "0",
    "deriv(x*y, z)", "deriv(x, z)*y + x*deriv(y, z)",
    "deriv(x/y, z)", "(deriv(x, z)*y - x*deriv(y, z)) / y^2",
    "deriv(sin(x), z)", "cos(x) * deriv(x, z)",
    "deriv(cos(x), z)", "-sin(x) * deriv(x, z)",
    "deriv(tan(x), z)", "deriv(x, z) * cos(x)^-2",
    "deriv(e^y, z)", "deriv(y, z)*e^y",
    "deriv(x^y, z)", "((y*deriv(x, z))*x^-1 + deriv(y, z)*ln(x))*x^y",
    "deriv(ln(x), z)", "deriv(x, z)*^-1",
};

#define NUM_NORMAL_FORM_RULES 5
RewriteRule normal_form_rules[NUM_NORMAL_FORM_RULES];
char *normal_form_strings[] = {
    "x-y", "x+(-y)",
    "-(x+y)", "-x+(-y)",
    "-(x*y)", "(-x)*y",
    "dX+cY", "cY+dX",
    "dX*cY", "cY*dX",
};

#define NUM_SIMPLIFICATION_RULES 42
RewriteRule simplification_rules[NUM_SIMPLIFICATION_RULES];
char *simplification_strings[] = {

    /* Get a nice sum */
    "x+y", "sum(x,y)",
    "sum([xs], sum([ys]), [zs])", "sum([xs], [ys], [zs])",
    "sum([xs])+sum([ys])", "sum([xs], [ys])",
    "x+sum([xs])", "sum(x, [xs])",
    "sum([xs])+x", "sum([xs], x)",
    //"sum([xs], dX, cY, [ys])", "sum([xs], cY, dX, [ys])", 

    /* Get a nice product */
    "x*y", "prod(x,y)",
    "prod([xs], prod([ys]), [zs])", "prod([xs], [ys], [zs])",
    "prod([xs])*prod([ys])", "prod([xs], [ys])",
    "x*prod([xs])", "prod(x, [xs])",
    "prod([xs])*x", "prod([xs], x)",
    //"prod([xs], dX, cY, [ys])", "prod([xs], cY, dX, [ys])", 

    /* Move constants and variables to the left */
    "sum([xs], dX, [ys], cY, [zs])", "sum(cY, [xs], dX, [ys], [zs])", // Constants left to variables or operators
    "sum([xs], oX, [ys], bY, [zs])", "sum([xs], bY, [ys], oX, [zs])", // Variables left to operators
    "prod([xs], dX, [ys], cY, [zs])", "prod(cY, [xs], dX, [ys], [zs])",
    "prod([xs], oX, [ys], bY, [zs])", "prod([xs], bY, [ys], oX, [zs])",

    /* Simplify sums */
    "sum()", "0",
    "sum(x)", "x",
    "sum([xs], 0, [ys])", "sum([xs], [ys])",
    "sum([xs], x, [ys], -x, [zs])", "sum([xs], [ys], [zs])",
    "sum([xs], -x, [ys], x, [zs])", "sum([xs], [ys], [zs])",

    /* Simplify products */
    "prod()", "1",
    "prod(x)", "x",
    "prod([xs], x, [ys], x, [zs])", "prod([xs], x^2, [ys], [zs])",
    "prod([xs], x, [ys], x^y, [zs])", "prod([xs], x^(y+1), [ys], [zs])",
    "prod([xs], x^y, [ys], x, [zs])", "prod([xs], x^(y+1), [ys], [zs])",
    "prod([xs], x^z, [ys], x^y, [zs])", "prod([xs], x^(y+z), [ys], [zs])",
    "prod([xs], 0, [ys])", "0",
    "prod([xs], 1, [ys])", "prod([xs], [ys])",
    "prod([xs], -x, [ys])", "-prod([xs], x, [ys])",

    /* Products within sum */
    "sum([xs], prod(a, x), [ys], x, [zs])", "sum([xs], prod(a+1, x), [ys], [zs])",
    "sum([xs], prod(x, a), [ys], x, [zs])", "sum([xs], prod(x, a+1), [ys], [zs])",
    "sum([xs], prod(b, x), [ys], prod(a, x), [zs])", "sum([xs], [ys], prod(a+b, x), [zs])",
    "sum([xs], prod(x, b), [ys], prod(x, a), [zs])", "sum([xs], [ys], prod(a+b, x), [zs])",
    "sum([xs], prod(b, x), [ys], prod(x, a), [zs])", "sum([xs], [ys], prod(a+b, x), [zs])",
    "sum([xs], prod(x, b), [ys], prod(a, x), [zs])", "sum([xs], [ys], prod(a+b, x), [zs])",
    "sum([xs], x, [ys], prod(a, x), [zs])", "sum([xs], [ys], prod(a+1, x), [zs])",
    "sum([xs], x, [ys], prod(x, a), [zs])", "sum([xs], [ys], prod(x, a+1), [zs])",
    "sum([xs], x, [ys], x, [zs])", "sum(prod(2, x), [xs], [ys], [zs])",
    "sum([xs], prod([yy], x, [zz]), [ys], prod([y], x, [z]), [zs])", "sum([xs], x * (prod([yy],[zz]) + prod([y],[z])), [ys], [zs])",

    /* Powers */
    "(x^y)^z", "x^prod(y, z)",
    "prod([xs], x^y, [ys], x^z, [zs])", "prod([xs], x^(y+z), [ys], [zs])",
    "x^0", "1",
    "prod(x, [xs])^z", "prod(x^z, prod([xs])^z)",
};

#define NUM_PRETTY_RULES 20
RewriteRule pretty_rules[NUM_PRETTY_RULES];
char *pretty_strings[] = 
{
    "prod([xs], sin(x), [ys], cos(x)^-1, [zs])", "prod([xs], tan(x), [ys], [zs])",
    "prod([xs], sin(x)^y, [ys], cos(x)^-y, [zs])", "prod([xs], tan(x)^y, [ys], [zs])",

    "-prod(x, [xs])", "prod(-x, [xs])",
    "-sum([xs], x)", "-sum([xs])-x",
    "prod([xs], x)^cY", "prod([xs])^cY * x^cY",

    "(x+y)^2", "x^2 + 2*x*y + y^2",
    "cX*(x+y)", "cX*x + cX*y",
    "x+(y+z)", "x+y+z",
    "x*(y*z)", "x*y*z",
    "sum(x, y)", "x+y",
    "sum([xs], x)", "sum([xs])+x",
    "sum(x)", "x",
    "prod(x)", "x",
    "prod(x, y)", "x*y",
    "prod([xs], x)", "prod([xs])*x",
    "--x", "x",
    "x+(-y)", "x-y",
    "x^1", "x",
    "root(x, 2)", "sqrt(x)",
    "x^(1/y)", "root(x, y)",
};

bool parse_rule(char *before, char *after, RewriteRule *out_rule)
{
    Node *before_n = parse_conveniently(g_ctx, before);
    if (before_n == NULL) 
    {
        printf("Syntax error in rulestring: %s\n", before);
        return false;
    }
    Node *after_n = parse_conveniently(g_ctx, after);
    if (after_n == NULL)
    {
        printf("Syntax error in rulestring: %s\n", after);
        return false;
    }
    *out_rule = get_rule(&before_n, &after_n);
    return true;
}

bool parse_rules(size_t num_rules, char **input, RewriteRule *out_rules)
{
    for (size_t i = 0; i < num_rules; i++)
    {
        if (!parse_rule(input[2 * i], input[2 * i + 1], &out_rules[i]))
        {
            return false;
        }
    }

    return true;
}

void replace_negative_consts(Node **tree)
{
    if (get_type(*tree) == NTYPE_CONSTANT)
    {
        if (get_const_value(*tree) < 0)
        {
            Node *minus_op = malloc_operator_node(ctx_lookup_op(g_ctx, "-", OP_PLACE_PREFIX), 1);
            set_child(minus_op, 0, malloc_constant_node(fabs(get_const_value(*tree))));
            tree_replace(tree, minus_op);
        }
    }
    else
    {
        if (get_type(*tree) == NTYPE_OPERATOR)
        {
            for (size_t i = 0; i < get_num_children(*tree); i++)
            {
                replace_negative_consts(get_child_addr(*tree, i));
            }
        }
    }
}

void init_simplification()
{
    deriv_before = P("x'");
    deriv_after = P("deriv(x, z)");
    malformed_derivA = P("deriv(x, cX)");
    malformed_derivB = P("deriv(x, oX)");
    parse_rules(NUM_OP_ELIMINATION_RULES, op_elimination_strings, op_elimination_rules);
    parse_rules(NUM_DERIVATION_RULES, derivation_rule_strings, derivation_rules);
    parse_rules(NUM_NORMAL_FORM_RULES, normal_form_strings, normal_form_rules);
    parse_rules(NUM_SIMPLIFICATION_RULES, simplification_strings, simplification_rules);
    parse_rules(NUM_PRETTY_RULES, pretty_strings, pretty_rules);
}

void unload_simplification()
{
    free_tree(deriv_before);
    free_tree(deriv_after);
    free_tree(malformed_derivA);
    free_tree(malformed_derivB);

    for (size_t i = 0; i < NUM_OP_ELIMINATION_RULES; i++)
    {
        free_rule(op_elimination_rules[i]);
    }
    for (size_t i = 0; i < NUM_DERIVATION_RULES; i++)
    {
        free_rule(derivation_rules[i]);
    }
    for (size_t i = 0; i < NUM_NORMAL_FORM_RULES; i++)
    {
        free_rule(normal_form_rules[i]);
    }
    for (size_t i = 0; i < NUM_SIMPLIFICATION_RULES; i++)
    {
        free_rule(simplification_rules[i]);
    }
    for (size_t i = 0; i < NUM_PRETTY_RULES; i++)
    {
        free_rule(pretty_rules[i]);
    }
}

/*
Summary: Applies all pre-defined rewrite rules to tree
Returns: True when transformations could be applied, False otherwise
*/
bool core_simplify(Node **tree)
{
    Matching matching;
    Node **matched;
    while ((matched = find_matching(tree, deriv_before, &matching)) != NULL)
    {
        char *var_name;
        size_t var_count = count_variables_distinct(*tree);

        if (var_count > 1)
        {
            report_error("You can only use expr' when there is not more than one variable in expr.\n");
            return false;
        }

        Node *replacement = tree_copy(deriv_after);
        if (var_count == 1)
        {
            list_variables(*tree, &var_name);
            tree_replace(get_child_addr(replacement, 1), P(var_name));
        }
        tree_replace(get_child_addr(replacement, 0), tree_copy(matching.mapped_nodes[0].nodes[0]));
        tree_replace(matched, replacement);
    }

    if (find_matching_discarded(*tree, malformed_derivA) != NULL
        || find_matching_discarded(*tree, malformed_derivA) != NULL)
    {
        report_error("Second operand of deriv must be variable.\n");
        return false;
    }

    apply_ruleset(tree, NUM_OP_ELIMINATION_RULES, op_elimination_rules);

    Node *tree_before = NULL;
    do
    {
        free_tree(tree_before);
        tree_before = tree_copy(*tree);

        for (size_t j = 0; j < NUM_DERIVATION_RULES; j++)
        {
            if (apply_rule(tree, &derivation_rules[j]))
            {
                break;
            }
        }

        apply_ruleset(tree, NUM_NORMAL_FORM_RULES, normal_form_rules);
        apply_ruleset(tree, NUM_SIMPLIFICATION_RULES, simplification_rules);
        replace_constant_subtrees(tree, false, op_evaluate);
        apply_ruleset(tree, NUM_PRETTY_RULES, pretty_rules);
        replace_constant_subtrees(tree, false, op_evaluate);
        replace_negative_consts(tree);
    } while (tree_compare(tree_before, *tree) != NULL);
    free_tree(tree_before);

    if (find_matching_discarded(*tree, deriv_after) != NULL)
    {
        report_error("Could not derivate expression.\n");
        return false;
    }

    return true;
}
