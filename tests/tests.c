#include <stdio.h>

#include "parser_test.h"
#include "tree_to_string_test.h"
#include "../src/engine/constants.h"

int main()
{
    int result = parser_test();
    if (result != 0) return -1;
    result = tree_to_string_test();
    if (result != 0) return -1;

    printf(F_GREEN "passed (%s)" COL_RESET "\n", VERSION);
    return 0;
}