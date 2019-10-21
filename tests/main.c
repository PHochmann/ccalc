#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "test.h"
#include "parser_test.h"
#include "tree_to_string_test.h"
#include "../src/commands/core.h"
#include "../src/arithmetics/arith_context.h"

static const size_t NUM_TESTS = 2;
static Test (*test_getters[])() = {
    get_parser_test,
    get_tree_to_string_test,
};

int main()
{
    arith_init_ctx();

    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Test test = test_getters[i]();
        printf("%s: ", test.name);
        if (test.suite())
        {
            printf("Passed %d cases.\n", test.num_cases);
        }
        else
        {
            goto error;
        }
    }

    printf(F_GREEN "All tests passed." COL_RESET "\n");
    return EXIT_SUCCESS;

    error:
    printf(F_RED "Build contains errors." COL_RESET "\n");
    return EXIT_FAILURE;
}
