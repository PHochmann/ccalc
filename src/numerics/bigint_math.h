#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "bigint.h"

BigInt *bigint_abs(BigInt *acc);
BigInt *bigint_invert(BigInt *acc);
BigInt *bigint_add(BigInt *acc, const BigInt *b);
BigInt *bigint_subtract(BigInt *acc, BigInt *b);

BigInt *bigint_shift_left(BigInt *acc, const size_t b);
BigInt *bigint_shift_right(BigInt *acc, const size_t b);
BigInt *bigint_and(BigInt *acc, const BigInt *b);
BigInt *bigint_or(BigInt *acc, const BigInt *b);
BigInt *bigint_xor(BigInt *acc, const BigInt *b);
BigInt *bigint_set_bit(BigInt *acc, size_t index);
BigInt *bigint_clear_bit(BigInt *acc, size_t index);

BigInt bigint_multiply_out_of_place(const BigInt *a, const BigInt *b);
BigInt *bigint_multiply(BigInt *acc, const BigInt *b);
BigInt *bigint_divide(BigInt *acc, BigInt *divisor, BigInt *out_remainder);
BigInt bigint_divide_out_of_place(BigInt *divident, BigInt *divisor, BigInt *out_remainder);
BigInt *bigint_factorial(BigInt *acc, const BigInt *b);
BigInt bigint_pow_out_of_place(const BigInt *base, size_t exponent);
BigInt *bigint_pow(BigInt *acc, size_t exponent);
BigInt *bigint_log(BigInt *acc, const BigInt *base);
BigInt bigint_log_out_of_place(const BigInt *a, const BigInt *base);

int bigint_comp(const BigInt *a, const BigInt *b);
int bigint_abscomp(const BigInt *a, const BigInt *b);
int bigint_compz(BigInt *a); // a is not const due to trimming
bool bigint_eq(const BigInt *a, const BigInt *b);
bool bigint_geq(const BigInt *a, const BigInt *b);
bool bigint_leq(const BigInt *a, const BigInt *b);
bool bigint_gz(const BigInt *a);
bool bigint_lz(const BigInt *a);
bool bigint_is_positive(const BigInt *a);
bool bigint_is_negative(const BigInt *a);
