#pragma once
#include <stdbool.h>

#define F_RED     "\x1B[1;31m"
#define F_GREEN   "\x1B[1;32m"
#define COL_RESET "\x1B[0m"

typedef struct {
    char *(*suite)(); // Returns NULL or error message
    int num_cases;
    char *name;
} Test;

char *create_error(char *fmt, ...);
