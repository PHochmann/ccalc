#include <stdlib.h>
#include <stdio.h>

#include "test_transformation.h"
#include "../src/tree/operator.h"
#include "../src/tree/tree_util.h"
#include "../src/tree/tree_to_string.h"

static const size_t NUM_CASES = 0;

bool transformation_test(__attribute__((unused)) StringBuilder *error_builder)
{
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
