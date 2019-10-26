#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../src/commands/core.h"
#include "../src/arithmetics/arith_context.h"
#include "../src/util/table.h"

#include "test.h"
#include "parser_test.h"
#include "tree_to_string_test.h"
#include "randomized_test.h"

static const size_t NUM_TESTS = 3;
static Test (*test_getters[])() = {
    get_parser_test,
    get_tree_to_string_test,
    get_randomized_test,
};

int main()
{
    arith_init_ctx();
    reset_table();
    add_cell(TEXTPOS_LEFT, "");
    add_cell(TEXTPOS_CENTER, " Test suite ");
    add_cell(TEXTPOS_CENTER, " #Cases ");
    add_cell(TEXTPOS_CENTER, " Result ");
    next_row();
    hline();

    bool error = false;
    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Test test = test_getters[i]();
        add_cell(TEXTPOS_LEFT, " %zu ", i);
        add_cell(TEXTPOS_LEFT, " %s ", test.name);
        add_cell(TEXTPOS_LEFT, " %d ", test.num_cases);
        if (test.suite())
        {
            add_cell(TEXTPOS_LEFT, F_GREEN " passed " COL_RESET, test.name);
        }
        else
        {
            add_cell(TEXTPOS_LEFT, F_RED " error " COL_RESET, test.name);
            error = true;
        }
        next_row();
    }

    print_table(true);
    reset_table();

    if (!error)
    {
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}
