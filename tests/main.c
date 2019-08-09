#include <stdio.h>
#include "test.h"
#include "parser_test.h"
#include "tree_to_string_test.h"
#include "../src/commands/core.h"
#include "../src/engine/constants.h"

#define NUM_TESTS 2

static Test (*test_getters[NUM_TESTS])() = {
    get_parser_test,
    get_tree_to_string_test,
};

int main()
{
    int total_cases = 0;

    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Test test = test_getters[i]();
        printf("%s:", test.name);

        int error_code = test.suite();
        if (error_code != 0)
        {
            printf(F_RED "Test returned %d" COL_RESET "\n", error_code);
            return error_code;
        }
        else
        {
            printf(" passed (%d)\n", test.num_cases);
            total_cases += test.num_cases;
        }
    }

    printf(F_GREEN "All tests passed (Version: %s, Cases: %d)" COL_RESET "\n", VERSION, total_cases);
    return 0;
}
