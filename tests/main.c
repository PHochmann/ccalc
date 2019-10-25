#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../src/commands/core.h"
#include "../src/arithmetics/arith_context.h"
#include "../src/util/table.h"

#include "test.h"
#include "parser_test.h"
#include "tree_to_string_test.h"
#include "massive_test.h"

static const size_t NUM_TESTS = 3;
static Test (*test_getters[])() = {
    get_parser_test,
    get_tree_to_string_test,
    get_massive_test,
};

int main()
{
    arith_init_ctx();
    reset_table();
    add_cell(TEXTPOS_LEFT_ALIGNED, "");
    add_cell(TEXTPOS_CENTERED, " Test Suite ");
    add_cell(TEXTPOS_CENTERED, " # Cases ");
    add_cell(TEXTPOS_CENTERED, " Result ");
    next_row();
    hline();

    bool error = false;
    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Test test = test_getters[i]();
        add_cell(TEXTPOS_LEFT_ALIGNED, " %zu ", i);
        add_cell(TEXTPOS_LEFT_ALIGNED, " %s ", test.name);
        add_cell(TEXTPOS_LEFT_ALIGNED, " %d ", test.num_cases);
        if (test.suite())
        {
            add_cell(TEXTPOS_LEFT_ALIGNED, " passed ", test.name);
        }
        else
        {
            add_cell(TEXTPOS_LEFT_ALIGNED, " error ", test.name);
            error = true;
        }
        next_row();
    }

    print_table(true);
    reset_table();

    if (!error)
    {
        printf(F_GREEN "All tests passed." COL_RESET "\n");
        return EXIT_SUCCESS;
    }
    else
    {
        printf(F_RED "Build contains errors." COL_RESET "\n");
        return EXIT_FAILURE;
    }
}
