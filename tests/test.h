#pragma once
#include <stdbool.h>
#include "../src/util/string_builder.h"

#define F_RED     "\x1B[1;31m"
#define F_GREEN   "\x1B[1;32m"
#define COL_RESET "\x1B[0m"

#define ERROR(...) {\
    strbuilder_append(error_builder, "line %d: ", __LINE__);\
    strbuilder_append(error_builder, __VA_ARGS__);\
    return false;\
}

#define ERROR_RETURN_VAL(function) ERROR("%s", "Unexpected return value of " function ".\n");

typedef struct {
    bool (*suite)(StringBuilder *error_builder);
    const char *name;
} Test;
