#pragma once
#include <stdbool.h>

#define F_RED     "\x1B[1;31m"
#define F_GREEN   "\x1B[1;32m"
#define COL_RESET "\x1B[0m"

typedef struct {
    bool (*suite)(); // Returns: True if test passed, otherwise false
    int num_cases;
    char *name;
} Test;
