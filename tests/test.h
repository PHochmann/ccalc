#pragma once
#include <stdbool.h>

#define COL_RESET "\033[0m"
#define F_RED     "\x1B[1;31m"
#define F_GREEN   "\x1B[1;32m"

typedef struct {
    bool (*suite)(); // Returns: True if test passed, otherwise false
    int num_cases;
    char *name;
} Test;
