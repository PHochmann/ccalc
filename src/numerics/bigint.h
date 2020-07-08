#pragma once
#include <stdbool.h>
#include <sys/types.h>
#include <stdint.h>

#include "../util/vector.h"

typedef struct
{
    bool is_positive;
    size_t data_count;
    size_t data_size;
    uint64_t *data;
} BigInt;

void bigint_dump(const BigInt *bigint);
void bigint_to_strbuilder(const BigInt *bigint, uint8_t base, Vector *builder);
bool bigint_from_string(const char *string, uint8_t base, BigInt *out);
void bigint_free(BigInt *bigint);
BigInt bigint_copy(const BigInt *bigint);
BigInt bigint_from_int(int32_t x);

BigInt bigint_testpattern();
