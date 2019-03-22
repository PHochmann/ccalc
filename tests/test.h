#pragma once

typedef struct {
    int (*suite)(); // Returns: 0 if all tests passed, error code otherwise
    int num_cases;
    char *name;
} Test;
