#include "../tree/tree_util.h"
#include "../parsing/parser.h"
#include "../transformation/rewrite_rule.h"
#include "simplification.h"
#include "arith_context.h"
#include "evaluation.h"
#include "../util/console_util.h"

#define P(x) parse_conveniently(g_ctx, x)

#define NUM_OP_ELIMINATION_RULES 18
RewriteRule op_elimination_rules[NUM_OP_ELIMINATION_RULES];
char *op_elimination_strings[] = {
    "$x", "x",
    "--x", "x",
    "x/y", "x*y^(-1)",
    "x+(y+z)", "x+y+z",
    "x*(y*z)", "x*y*z",
    "log(x, e)", "ln(x)",

    "deriv(cX*y^cZ, y)", "cZ*cX*y^(cZ-1)",
    "deriv(-x, z)", "-deriv(x, z)",
    "deriv(x + y, z)", "deriv(x, z) + deriv(y, z)",
    "deriv(x - y, z)", "deriv(x, z) - deriv(y, z)",
    "deriv(x, x)", "1",
    "deriv(vX, z)", "0",
    "deriv(cX, z)", "0",
    "deriv(x*y, z)", "deriv(x, z)*y + x*deriv(y, z)",
    "deriv(x/y, z)", "(deriv(x, z)*y - x*deriv(y, z)) / y^2",
    //"deriv(x^y, z)", "deriv(x, z)^y * deriv(y, z)",

    "deriv(sin(x), z)", "cos(x) * deriv(x, z)",
    "deriv(cos(x), z)", "-sin(x) * deriv(x, z)",

    "deriv(x^y, z)", "((y*deriv(x, z))/x + deriv(y, z)*ln(x))*x^y"
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

#define NUM_SIMPLIFICATION_RULES 33
RewriteRule simplification_rules[NUM_SIMPLIFICATION_RULES];
char *simplification_strings[] = {

    /* Move constants */
    "sum([xs], vx, [ys], cx, [zs])", "sum(cx, [xs], vx, [ys], [zs])", 

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

    /* Simplify sums */
    "sum(x)", "x",
    "sum([xs], 0, [ys])", "sum([xs], [ys])",
    "sum([xs], x, [ys], -x, [zs])", "sum([xs], [ys], [zs])",
    "sum([xs], -x, [ys], x, [zs])", "sum([xs], [ys], [zs])",

    /* Simplify products */
    "prod(x)", "x",
    "prod([xs], x, [ys], x, [zs])", "prod([xs], x^2, [ys], [zs])",
    "prod([xs], x, [ys], x^y, [zs])", "prod([xs], x^(y+1), [ys], [zs])",
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
    "(x^y)^z", "x^(y*z)",
};

#define NUM_PRETTY_RULES 11
RewriteRule pretty_rules[NUM_PRETTY_RULES];
char *pretty_strings[] = {
    /* Move constants into products */
    "cX*(x+y)", "cX*x + cX*y",
    "x+(y+z)", "x+y+z",
    "x*(y*z)", "x*y*z",
    "sum(x, y)", "x+y",
    "sum(x, [xs])", "x+sum([xs])",
    "sum(x)", "x",
    "prod(x)", "x",
    "prod(x, y)", "x*y",
    "prod(x, [xs])", "x*prod([xs])",
    "--x", "x",
    "x^1", "x",
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

void init_simplification()
{
    parse_rules(NUM_OP_ELIMINATION_RULES, op_elimination_strings, op_elimination_rules);
    parse_rules(NUM_NORMAL_FORM_RULES, normal_form_strings, normal_form_rules);
    parse_rules(NUM_SIMPLIFICATION_RULES, simplification_strings, simplification_rules);
    parse_rules(NUM_PRETTY_RULES, pretty_strings, pretty_rules);
}

void unload_simplification()
{
    for (size_t i = 0; i < NUM_OP_ELIMINATION_RULES; i++)
    {
        free_rule(op_elimination_rules[i]);
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
    while ((matched = find_matching(tree, P("x'"), &matching)) != NULL)
    {
        char *var_name;
        size_t var_count = count_variables_distinct(*tree);

        if (var_count > 1)
        {
            report_error("You can only use expr' when there is not more than one variable in expr.\n");
            return false;
        }

        Node *deriv_node = P("deriv(x, z)");
        if (var_count == 1)
        {
            list_variables(*tree, &var_name);
            tree_replace(get_child_addr(deriv_node, 1), P(var_name));
        }
        tree_replace(get_child_addr(deriv_node, 0), tree_copy(matching.mapped_nodes[0].nodes[0]));
        tree_replace(matched, deriv_node);
    }

    if (find_matching_discarded(*tree, P("deriv(x, wZ)")) != NULL)
    {
        report_error("Second operand of deriv must be variable.\n");
        return false;
    }

    apply_ruleset(tree, NUM_OP_ELIMINATION_RULES, op_elimination_rules);
    replace_constant_subtrees(tree, op_evaluate);

    if (find_matching_discarded(*tree, P("deriv(x, y)")) != NULL)
    {
        report_error("Could not derivate expression.\n");
        return false;
    }

    replace_constant_subtrees(tree, op_evaluate);

    Node *tree_before = NULL;
    do
    {
        free_tree(tree_before);
        tree_before = tree_copy(*tree);
        apply_ruleset(tree, NUM_NORMAL_FORM_RULES, normal_form_rules);
        replace_constant_subtrees(tree, op_evaluate);
        apply_ruleset(tree, NUM_SIMPLIFICATION_RULES, simplification_rules);
        replace_constant_subtrees(tree, op_evaluate);
        apply_ruleset(tree, NUM_PRETTY_RULES, pretty_rules);
        replace_constant_subtrees(tree, op_evaluate);
    } while (tree_compare(tree_before, *tree) != NULL);

    return true;
}
