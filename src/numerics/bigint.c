#include <stdio.h>
#include <string.h>
#include <math.h>
#include "bigint.h"
#include "bigint_math.h"
#include "../util/string_util.h"

BigInt create_bigint(size_t data_count)
{
    if (data_count == 0) printf("Error: create_bigint called with data_count=0.\n");
    BigInt res;
    res.is_positive = true;
    res.data_count = data_count;
    res.data_size = data_count;
    res.data = calloc(data_count, sizeof(uint64_t));
    return res;
}

char digit_to_char(uint64_t d)
{
    if (d <= 9) return '0' + d;
    if (d >= 10 && d < 16) return 'a' + d;
    return '?';
}

#define INVALID_CHAR 100
uint64_t char_to_digit(char c)
{
    if (c >= '0' && c <= '9') return (uint64_t)c - '0';
    if (c >= 'a' && c <= 'f') return (uint64_t)c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return (uint64_t)c - 'A' + 10;
    return INVALID_CHAR;
}

/* End Auxiliary */

void bigint_dump(const BigInt *bigint)
{
    if (!bigint->is_positive) printf("-");
    for (size_t i = 0; i < bigint->data_count; i++)
    {
        printf("%016lx ", bigint->data[bigint->data_count - 1 - i]);
    }
    printf("\n");
}

void bigint_to_strbuilder(const BigInt *bigint, uint8_t base, Vector *builder)
{
    BigInt copy = bigint_copy(bigint);
    BigInt big_base = bigint_from_int((int32_t)base);
    
    while (true)
    {
        BigInt remainder;
        bigint_divide(&copy, &big_base, &remainder);
        strbuilder_append_char(builder, digit_to_char(remainder.data[0]));
        bigint_free(&remainder);
        if (bigint_compz(&copy) == 0) break;
    }

    if (!bigint->is_positive)
    {
        strbuilder_append_char(builder, '-');
    }

    bigint_free(&copy);
    bigint_free(&big_base);

    strbuilder_reverse(builder);
}

/*
Params
    out: Is allowed to be NULL
*/
bool bigint_from_string(const char *string, uint8_t base, BigInt *out)
{
    // Super-early out for variable token case
    if (char_to_digit(string[0]) == INVALID_CHAR) return false;
    if (base > 16) return false;

    size_t len = strlen(string);
    BigInt res = bigint_from_int(0);
    BigInt big_base = bigint_from_int(base);
    BigInt b = bigint_from_int(1);

    ssize_t pos = len - 1;
    while (pos >= 0)
    {
        uint64_t digit = char_to_digit(string[pos]);
        if (digit >= base)
        {
            bigint_free(&res);
            bigint_free(&big_base);
            bigint_free(&b);
            return false;
        }

        BigInt big_digit = bigint_from_int(digit);
        bigint_multiply(&big_digit, &b);
        bigint_add(&res, &big_digit);
        bigint_free(&big_digit);
        bigint_multiply(&b, &big_base);
        pos--;
    }

    bigint_free(&big_base);
    bigint_free(&b);

    if (out != NULL)
    {
        *out = res;
    }
    else
    {
        bigint_free(&res);
    }
    
    return true;
}

void bigint_free(BigInt *bigint)
{
    if (bigint == NULL) return;
    free(bigint->data);
    bigint->data_count = 0;
    bigint->data = NULL;
}

BigInt bigint_copy(const BigInt *bigint)
{
    BigInt res = create_bigint(bigint->data_count);
    res.is_positive = bigint->is_positive;
    res.data_count = bigint->data_count;
    for (size_t i = 0; i < bigint->data_count; i++) res.data[i] = bigint->data[i];
    return res;
}

BigInt bigint_from_int(int32_t x)
{
    BigInt res = create_bigint(1);
    if (x < 0) res.is_positive = false;
    x = abs(x);
    res.data[0] = x;
    return res;
}

BigInt bigint_testpattern()
{
    BigInt res = create_bigint(3);
    res.data[0] = 0xF0F0F0F0F0F0F0F0;
    res.data[1] = 0xE0E0E0E0E0E0E0E0;
    res.data[2] = 0xD0D0D0D0D0D0D0D0;
    return res;
}
