#include <stdlib.h>
#include <stdio.h>

#include "test_node.h"
#include "../src/tree/operator.h"
#include "../src/tree/tree_util.h"
#include "../src/tree/tree_to_string.h"

static const size_t NUM_CASES = 0;

char *transformation_test()
{
    return NULL;
}

Test get_transformation_test()
{
    return (Test){
        transformation_test,
        NUM_CASES,
        "Transformation"
    };
}
