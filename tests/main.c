#include <stdio.h>
#include "test.h"
#include "parser_test.h"
#include "tree_to_string_test.h"
#include "../src/commands/core.h"

#define COL_RESET "\033[0m"
#define F_RED     "\x1B[1;31m"
#define F_GREEN   "\x1B[1;32m"

static const size_t NUM_TESTS = 2;
static Test (*test_getters[])() = {
    get_parser_test,
    get_tree_to_string_test,
};

int main()
{
    int total_cases = 0;

    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Test test = test_getters[i]();
        int error_code = test.suite();

        if (error_code != 0)
        {
            printf("%s: " F_RED "Test returned %d" COL_RESET "\n", test.name, error_code);
            return error_code;
        }
        else
        {
            printf("%s: passed all %d cases\n", test.name, test.num_cases);
            total_cases += test.num_cases;
        }
    }

    printf(F_GREEN "All tests passed (Version: %s, Cases: %d)" COL_RESET "\n", VERSION, total_cases);
    return 0;
}
