#include <stdlib.h>
#include <stdio.h>

#include "test_transformation.h"
#include "../src/tree/operator.h"
#include "../src/tree/tree_util.h"
#include "../src/tree/tree_to_string.h"

static const size_t NUM_CASES = 0;

bool transformation_test(__attribute__((unused)) Vector *error_builder)
{
    // 2*(sqrt(2)*x)^2 -> 4x^2
    return true;
}

Test get_transformation_test()
{
    return (Test){
        transformation_test,
        NUM_CASES,
        "Transformation"
    };
}
