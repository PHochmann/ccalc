#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "bigint_math.h"

/* Auxiliary */

uint64_t max(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}

// Don't use right after overwrite!
void overwrite(BigInt *left, BigInt *right)
{
    free(left->data);
    left->data = right->data;
    left->data_count = right->data_count;
    left->data_size = right->data_size;
    left->is_positive = right->is_positive;
    right->data_count = 0;
    right->data = NULL;
}

void ensure_size(BigInt *bigint, size_t new_size)
{
    size_t new_data_size = bigint->data_size;
    while (new_data_size < new_size)
    {
        new_data_size <<= 1;
    }

    bigint->data = realloc(bigint->data, new_data_size * sizeof(uint64_t));
    for (size_t i = bigint->data_size; i < new_data_size; i++) bigint->data[i] = 0;
    bigint->data_count = new_size;
    bigint->data_size = new_data_size;
}

void trim(BigInt *bigint)
{
    size_t new_size = bigint->data_count;
    while (new_size > 1)
    {
        if (bigint->data[new_size - 1] == 0)
        {
            new_size--;
        }
        else
        {
            break;
        }
    }
    ensure_size(bigint, new_size);
}

void normalize(BigInt *bigint)
{
    trim(bigint);
    if (bigint_compz(bigint) == 0) bigint->is_positive = true;
}

uint64_t get_data(const BigInt *bigint, size_t index)
{
    return (bigint->data_count > index) ? bigint->data[index] : 0;
}

uint64_t *copy_data(const BigInt *bigint)
{
    uint64_t *res = malloc(bigint->data_count * sizeof(uint64_t));
    for (size_t i = 0; i < bigint->data_count; i++) res[i] = bigint->data[i];
    return res;
}

size_t highest_set_bit(BigInt *bigint)
{
    trim(bigint);
    return 63 - (size_t)__builtin_clzl(bigint->data[bigint->data_count - 1]) + (bigint->data_count - 1) * 64;
}

int highest_set_bit_sizet(size_t x)
{
    return 63 - __builtin_clzl(x);
}

// Ensure acc >= b
void minus(uint64_t *acc, const uint64_t *b, size_t b_size)
{
    bool underflow = false;
    for (size_t i = 0; i < b_size; i++)
    {
        uint64_t sum = acc[i] - b[i];
        uint64_t sum_after_carry = sum - (underflow ? 1 : 0);
        underflow = (sum > acc[i] || (sum_after_carry > sum));
        acc[i] = sum_after_carry;
    }
    if (underflow) acc[b_size]--;
}

// Ensure size of acc 
void plus(uint64_t *acc, const uint64_t *b, size_t b_size)
{
    bool overflow = false;
    for (size_t i = 0; i < b_size; i++)
    {
        uint64_t sum = acc[i] + b[i];
        uint64_t sum_after_carry = sum + (overflow ? 1 : 0);
        overflow = (sum < acc[i] || (sum_after_carry < sum));
        acc[i] = sum_after_carry;
    }
    // Carry
    if (overflow) acc[b_size]++;
}

/* End Auxiliary */

BigInt *bigint_abs(BigInt *acc)
{
    acc->is_positive = true;
    return acc;
}

BigInt *bigint_invert(BigInt *acc)
{
    if (!bigint_compz(acc) == 0) acc->is_positive = !acc->is_positive;
    return acc;
}

BigInt *bigint_add(BigInt *acc, const BigInt *b)
{
    if (b->data_count + 1 > acc->data_count) ensure_size(acc, b->data_count + 1);
    if ((acc->is_positive && b->is_positive) || (!acc->is_positive && !b->is_positive))
    {
        plus(acc->data, b->data, b->data_count);
    }
    else
    {
        if (bigint_abscomp(acc, b) != -1)
        {
            minus(acc->data, b->data, b->data_count);
        }
        else
        {
            acc->is_positive = !acc->is_positive;
            uint64_t *copy_of_b = copy_data(b);
            minus(copy_of_b, acc->data, b->data_count);
            free(acc->data);
            acc->data = copy_of_b;
            acc->data_count = b->data_count;
            acc->data_size = b->data_count;
        }
    }

    normalize(acc);
    return acc;
}

BigInt *bigint_subtract(BigInt *acc, BigInt *b)
{
    bigint_invert(b);
    bigint_add(acc, b);
    bigint_invert(b);
    return acc;
}

BigInt *bigint_inc(BigInt *acc)
{
    uint64_t data = 1;
    BigInt one = { .is_positive = true, .data_count = 1, .data = &data };
    return bigint_add(acc, &one);
}

BigInt *bigint_dec(BigInt *acc)
{
    uint64_t data = 1;
    BigInt one = { .is_positive = false, .data_count = 1, .data = &data };
    return bigint_add(acc, &one);
}

BigInt *bigint_shift_left(BigInt *acc, size_t b)
{
    size_t index_shift = b / 64;
    size_t old_size = acc->data_count;
    ensure_size(acc, acc->data_count + index_shift + 1);
    // First: Shift for multiples of 64
    if (index_shift > 0)
    {
        // Copy data over from left to right
        for (ssize_t i = old_size; i >= 0; i--)
        {
            acc->data[i + index_shift] = acc->data[i];
        }
        // Set lowest cells to zero
        for (size_t i = 0; i < index_shift; i++)
        {
            acc->data[i] = 0;
        }
    }
    // Then do rest
    size_t bit_shift = b % 64;
    if (bit_shift > 0)
    {
        for (ssize_t i = acc->data_count - 2; i >= 0; i--)
        {
            acc->data[i + 1] |= acc->data[i] >> ((size_t)64 - bit_shift);
            acc->data[i] = acc->data[i] << bit_shift;
        }
    }
    trim(acc);
    return acc;
}

BigInt *bigint_shift_right(BigInt *acc, size_t b)
{
    size_t index_shift = b / 64;
    // First: Shift for multiples of 64
    if (index_shift > 0)
    {
        // Copy data over from right to left
        for (size_t i = 0; i < acc->data_count - index_shift; i++)
        {
            acc->data[i] = acc->data[i + index_shift];
        }
        // Set highest cells to zero
        for (size_t i = 0; i < index_shift; i++)
        {
            acc->data[acc->data_count - 1 - i] = 0;
        }
    }
    // Then do rest
    size_t bit_shift = b % 64;
    if (bit_shift > 0)
    {
        acc->data[0] = acc->data[0] >> bit_shift;
        for (size_t i = 1; i < acc->data_count; i++)
        {
            acc->data[i - 1] |= acc->data[i] << ((size_t)64 - bit_shift);
            acc->data[i] = acc->data[i] >> bit_shift;
        }
    }
    normalize(acc);
    return acc;
}

BigInt *bigint_and(BigInt *acc, const BigInt *b)
{
    size_t size = max(acc->data_count, b->data_count);
    ensure_size(acc, size);
    for (size_t i = 0; i < size; i++) acc->data[i] = get_data(acc, i) & get_data(b, i);
    return acc;
}

BigInt *bigint_or(BigInt *acc, const BigInt *b)
{
    size_t size = max(acc->data_count, b->data_count);
    ensure_size(acc, size);
    for (size_t i = 0; i < size; i++) acc->data[i] = get_data(acc, i) | get_data(b, i);
    return acc;
}

BigInt *bigint_xor(BigInt *acc, const BigInt *b)
{
    size_t size = max(acc->data_count, b->data_count);
    ensure_size(acc, size);
    for (size_t i = 0; i < size; i++) acc->data[i] = get_data(acc, i) ^ get_data(b, i);
    return acc;
}

BigInt *bigint_set_bit(BigInt *acc, size_t index)
{
    size_t offset = index / 64;
    if (offset >= acc->data_count) ensure_size(acc, offset + 1);
    acc->data[offset] |= ((uint64_t)1 << (index % 64));
    return acc;
}

BigInt *bigint_clear_bit(BigInt *acc, size_t index)
{
    size_t offset = index / 64;
    if (offset >= acc->data_count) return acc;
    acc->data[offset] &= ~((uint64_t)1 << (index % 64));
    return acc;
}

/* Multiplication and Division */

BigInt bigint_multiply_out_of_place(const BigInt *a, const BigInt *b)
{
    BigInt res = bigint_from_int(0);
    for (size_t i = 0; i < b->data_count; i++)
    {
        for (size_t j = 0; j < 64; j++)
        {
            if ((b->data[i] & ((size_t)1 << j)) != 0)
            {
                BigInt a_copy = bigint_copy(a);
                bigint_shift_left(&a_copy, i * 64 + j);
                bigint_add(&res, &a_copy);
                bigint_free(&a_copy);
            }
        }
    }
    res.is_positive = !(a->is_positive ^ b->is_positive);
    normalize(&res);
    return res;
}

BigInt *bigint_multiply(BigInt *acc, const BigInt *b)
{
    BigInt res = bigint_multiply_out_of_place(acc, b);
    overwrite(acc, &res);
    return acc;
}

BigInt *bigint_pow(BigInt *acc, size_t exponent)
{
    BigInt res = bigint_pow_out_of_place(acc, exponent);
    overwrite(acc, &res);
    return acc;
}

BigInt bigint_pow_out_of_place(const BigInt *base, size_t exponent)
{
    BigInt res = bigint_from_int(1);
    if (exponent == 0) return res;
    for (int j = highest_set_bit_sizet(exponent); j >= 0; j--)
    {
        bigint_multiply(&res, &res);
        if ((exponent & ((size_t)1 << (size_t)j)) != 0)
        {
            bigint_multiply(&res, base);
        }
    }
    return res;
}

// out_remainder is allowed to be NULL
BigInt *bigint_divide(BigInt *acc, BigInt *divisor, BigInt *out_remainder)
{
    BigInt res = bigint_divide_out_of_place(acc, divisor, out_remainder);
    overwrite(acc, &res);
    return acc;
}

// out_remainder is allowed to be NULL
BigInt bigint_divide_out_of_place(BigInt *divident, BigInt *divisor, BigInt *out_remainder)
{
    BigInt res = bigint_from_int(0);

    if (bigint_abscomp(divident, divisor) == -1)
    {
        *out_remainder = bigint_copy(divident);
        return res;
    }

    BigInt copy_of_divident = bigint_copy(divident);
    bigint_abs(&copy_of_divident);
    BigInt curr_divisor = bigint_copy(divisor);
    bigint_abs(&curr_divisor);
    ssize_t shift = highest_set_bit(divident) - highest_set_bit(divisor);
    bigint_shift_left(&curr_divisor, shift);

    while (shift >= 0)
    {
        if (bigint_abscomp(&copy_of_divident, &curr_divisor) != -1)
        {
            bigint_subtract(&copy_of_divident, &curr_divisor);
            bigint_set_bit(&res, shift);
        }
        
        bigint_shift_right(&curr_divisor, 1);
        shift--;
    }

    bigint_free(&curr_divisor);
    if (out_remainder != NULL)
    {
        copy_of_divident.is_positive = divident->is_positive;
        trim(&copy_of_divident);
        *out_remainder = copy_of_divident;
    }
    else
    {
        bigint_free(&copy_of_divident);
    }

    res.is_positive = !(divident->is_positive ^ divisor->is_positive);
    normalize(&res);
    return res;
}

/* Logarithm */

BigInt *bigint_log(BigInt *acc, const BigInt *base)
{
    BigInt res = bigint_log_out_of_place(acc, base);
    overwrite(acc, &res);
    return acc;
}

BigInt bigint_log_out_of_place(const BigInt *a, const BigInt *base)
{
    BigInt res = bigint_from_int(0);
    BigInt pow = bigint_from_int(1);
    while (true)
    {
        int comp = bigint_comp(&pow, a);
        if (comp == 1)
        {
            bigint_dec(&res);
            break;
        }
        if (comp == 0) break;

        bigint_inc(&res);
        bigint_multiply(&pow, base);
    }
    trim(&res);
    return res;
}

/* Comparison */

int bigint_comp(const BigInt *a, const BigInt *b)
{
    if (a->is_positive && !b->is_positive) return 1;
    if (b->is_positive && !a->is_positive) return -1;
    int abscomp = bigint_abscomp(a, b);
    if (a->is_positive && b->is_positive) return abscomp;
    if (!a->is_positive && !b->is_positive) return abscomp * -1;
    return 2;
}

int bigint_abscomp(const BigInt *a, const BigInt *b)
{
    for (ssize_t i = max(a->data_count, b->data_count) - 1; i >= 0; i--)
    {
        if (get_data(a, i) > get_data(b, i)) return 1;
        if (get_data(b, i) > get_data(a, i)) return -1;
    }
    return 0;
}

bool bigint_eq(const BigInt *a, const BigInt *b)
{
    return bigint_comp(a, b) == 0;
}

bool bigint_geq(const BigInt *a, const BigInt *b)
{
    return bigint_comp(a, b) >= 0;
}

bool bigint_leq(const BigInt *a, const BigInt *b)
{
    return bigint_comp(a, b) <= 0;
}

int bigint_compz(BigInt *a)
{
    trim(a);
    if (a->data_count > 1 || a->data[0] > 0)
    {
        return a->is_positive ? 1 : -1;
    }
    return 0;
}

bool bigint_is_positive(const BigInt *a)
{
    return a->is_positive;
}

bool bigint_is_negative(const BigInt *a)
{
    return !a->is_positive;
}
