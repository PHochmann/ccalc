#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "test.h"
#include "test_parser.h"
#include "test_tree_to_string.h"
#include "test_randomized.h"
#include "test_table.h"
#include "../src/commands/core.h"
#include "../src/arithmetics/arith_context.h"
#include "../src/table/table.h"

#if TEST_TABLES
static const size_t NUM_TESTS = 4;
#else
static const size_t NUM_TESTS = 3;
#endif

static Test (*test_getters[])() = {
    get_parser_test,
    get_tree_to_string_test,
    get_randomized_test,
    get_table_test,
};

int main()
{
    arith_init_ctx();
    Table table = get_empty_table();
    add_empty_cell(&table);
    add_cell(&table, get_settings_align_span_border(ALIGN_CENTER, 1, 1, BORDER_NONE, BORDER_SINGLE), " Test suite ");
    add_cell(&table, get_settings_align(ALIGN_CENTER), " #Cases ");
    add_cell(&table, get_settings_align(ALIGN_CENTER), " Result ");
    next_row(&table);

    bool error = false;
    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Test test = test_getters[i]();
        add_cell_fmt(&table, get_settings_align(ALIGN_LEFT), " %zu ", i + 1);
        add_cell_fmt(&table, get_settings_align(ALIGN_LEFT), " %s ", test.name);
        add_cell_fmt(&table, get_settings_align(ALIGN_RIGHT), " %d ", test.num_cases);
        if (test.suite())
        {
            add_cell(&table, get_settings_align(ALIGN_LEFT), F_GREEN " passed " COL_RESET);
        }
        else
        {
            add_cell(&table, get_settings_align(ALIGN_LEFT), F_RED " failed " COL_RESET);
            error = true;
        }
        next_row(&table);
    }
    add_cell(&table, get_settings_align_span(ALIGN_CENTER, 3, 1), "Overall result");
    add_standard_cell(&table, error ? F_RED " failed " COL_RESET : F_GREEN " passed " COL_RESET);

    horizontal_line(&table, BORDER_SINGLE, 2, 1, NUM_TESTS + 1);
    vertical_line(&table, BORDER_SINGLE, 1, 1);
    change_settings_at(&table, 1, 0, get_settings_align_span_border(ALIGN_LEFT, 1, 1, BORDER_NONE, BORDER_NONE));
    make_boxed(&table, BORDER_SINGLE);
    print_table(&table);
    free_table(&table);

    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
