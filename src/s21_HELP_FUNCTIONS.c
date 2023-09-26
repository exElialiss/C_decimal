#include "s21_decimal.h"

s21_decimal ten = {{10, 0, 0, 0}};
s21_decimal decimal_null = {{0, 0, 0, 0}};
s21_decimal one_decimal = {{1, 0, 0, 0}};
s21_big_decimal one_big_decimal = {{1, 0, 0, 0, 0, 0, 0}};
s21_big_decimal big_ten = {{10, 0, 0, 0, 0, 0, 0}};
s21_big_decimal big_decimal_null = {{0, 0, 0, 0, 0, 0, 0}};

int difference(s21_big_decimal value_1, s21_big_decimal value_2, s21_big_decimal *result)
{
    *result = value_1;
    for (int bit = 0; bit <= 191; bit++)
    {
        if (big_getBit(*result, bit) && big_getBit(value_2, bit))
        {
            big_setBit(result, bit, 0);
        }
        else if (!big_getBit(*result, bit) && big_getBit(value_2, bit))
        {
            int count_x = bit;
            while (!big_getBit(*result, count_x))
            {
                count_x++;
            }
            big_setBit(result, bit, 1);
            while (bit != count_x)
            {
                bit++;
                if (bit != count_x)
                {
                    if (big_getBit(value_2, bit))
                    {
                        big_setBit(result, bit, 0);
                    }
                    else
                    {
                        big_setBit(result, bit, 1);
                    }
                }
            }
            big_setBit(result, bit, 0);
            bit--;
        }
    }
    return 0;
}
int getBit(s21_decimal value, int bit)
{
    return !!(value.bits[bit / 32] & (1 << (bit % 32)));
}
int setBit(s21_decimal *value, int bit, int num)
{
    if (num == 1)
    {
        value->bits[bit / 32] = value->bits[bit / 32] | (num << (bit % 32));
    }
    if (num == 0)
    {
        value->bits[bit / 32] = value->bits[bit / 32] & (~(1 << (bit % 32)));
    }
    return 0;
}
int find_out_the_degree(s21_big_decimal scale)
{
    int mask = 0b11111111 << 16;
    return (scale.bits[6] & mask) >> 16;
}

int degree_comparison(s21_big_decimal value_1, s21_big_decimal value_2)
{
    int scale_of_value_1 = 0;
    int scale_of_value_2 = 0;
    int result = 6;
    scale_of_value_1 = find_out_the_degree(value_1);
    scale_of_value_2 = find_out_the_degree(value_2);
    if (scale_of_value_1 > scale_of_value_2)
    {
        result = 1;
    }
    else if (scale_of_value_1 < scale_of_value_2)
    {
        result = -1;
    }
    else if (scale_of_value_1 == scale_of_value_2)
    {
        result = 0;
    }
    return result;
}
void normalization(s21_big_decimal *value_1, s21_big_decimal *value_2)
{
    if (degree_comparison(*value_1, *value_2) == 1)
    {
        if (find_out_the_degree(*value_1) <= 28)
        {
            while (find_out_the_degree(*value_2) != find_out_the_degree(*value_1))
            {
                raise_scale(value_2);
            }
        }
        else
        {
            while (find_out_the_degree(*value_1) != 28)
            {
                demotion_scale(value_1);
            }
            if (degree_comparison(*value_1, *value_2) == 1)
            {
                while (find_out_the_degree(*value_2) != find_out_the_degree(*value_1))
                {
                    raise_scale(value_2);
                }
            }
        }
    }
    else if (degree_comparison(*value_1, *value_2) == -1)
    {
        if (find_out_the_degree(*value_2) <= 28)
        {
            while (find_out_the_degree(*value_1) != find_out_the_degree(*value_2))
            {
                raise_scale(value_1);
            }
        }
        else
        {
            while (find_out_the_degree(*value_2) != 28)
            {
                demotion_scale(value_2);
            }
            if (degree_comparison(*value_1, *value_2) == -1)
            {
                while (find_out_the_degree(*value_1) != find_out_the_degree(*value_2))
                {
                    raise_scale(value_1);
                }
            }
        }
    }
}
void big_uninitilization(s21_big_decimal big_value, s21_decimal *smal_value)
{
    smal_value->bits[0] = big_value.bits[0];
    smal_value->bits[1] = big_value.bits[1];
    smal_value->bits[2] = big_value.bits[2];
    smal_value->bits[3] = big_value.bits[6];
}
int big_getBit(s21_big_decimal value, int bit)
{
    if (bit <= 223)
    {
        return !!(value.bits[bit / 32] & (1 << (bit % 32)));
    }
    return 0;
}
int big_setBit(s21_big_decimal *value, int bit, int num)
{
    if (bit <= 191 && bit >= 0)
    {
        if (num == 1)
        {
            value->bits[bit / 32] = value->bits[bit / 32] | (num << (bit % 32));
        }
        if (num == 0)
        {
            value->bits[bit / 32] = value->bits[bit / 32] & (~(1 << (bit % 32)));
        }
    }
    return 0;
}
int demotion_scale(s21_big_decimal *value)
{
    int error = 0;
    unsigned int tmp_bit = value->bits[6] >> 31;
    big_setBit(value, 223, 0);
    unsigned int num = value->bits[6] >> 16;
    if (!num)
    {
        error = 1;
    }
    num--;
    value->bits[6] = num << 16;
    big_setBit(value, 223, tmp_bit);
    s21_big_decimal copy_value = *value;
    division_by_integer(*value, big_ten, &copy_value);
    *value = copy_value;
    return error;
}
int raise_scale(s21_big_decimal *value)
{
    int error = 0;
    int error_mult = 0;
    unsigned int tmp_bit = value->bits[6] >> 31;
    big_setBit(value, 223, 0);
    unsigned int num = value->bits[6] >> 16;
    if (num == 28)
    {
        error = 1;
    }
    num++;
    value->bits[6] = num << 16;
    big_setBit(value, 223, tmp_bit);
    s21_big_decimal copy_value = *value;
    error_mult = multiplication(*value, big_ten, &copy_value);
    *value = copy_value;
    if (error_mult)
    {
        error = 1;
    }
    return error;
}
int big_addition(s21_big_decimal current_value_1, s21_big_decimal current_value_2, s21_big_decimal *big_result)
{
    int error = 0;
    for (int bit = 0; bit <= 191; bit++)
    {
        if (big_getBit(current_value_1, bit) && (!big_getBit(current_value_2, bit)))
        {
            big_setBit(big_result, bit, 1);
        }
        else if (big_getBit(current_value_2, bit) && (!big_getBit(current_value_1, bit)))
        {
            big_setBit(big_result, bit, 1);
        }
        else if (big_getBit(current_value_1, bit) && big_getBit(current_value_2, bit))
        {
            big_setBit(big_result, bit, 0);
            while ((big_getBit(current_value_1, bit)) || (big_getBit(current_value_2, bit)))
            {
                bit++;
                if (bit > 95)
                {
                    error = 1;
                }
                if ((big_getBit(current_value_1, bit) && (!big_getBit(current_value_2, bit))) || (big_getBit(current_value_2, bit) && (!big_getBit(current_value_1, bit))))
                {
                    big_setBit(big_result, bit, 0);
                }
                else if (big_getBit(current_value_1, bit) && big_getBit(current_value_2, bit))
                {
                    big_setBit(big_result, bit, 1);
                }
            }
            big_setBit(big_result, bit, 1);
        }
    }
    return error;
}
void big_initialization(s21_decimal small_value, s21_big_decimal *big_value)
{
    big_value->bits[0] = small_value.bits[0];
    big_value->bits[1] = small_value.bits[1];
    big_value->bits[2] = small_value.bits[2];
    big_value->bits[3] = 0;
    big_value->bits[4] = 0;
    big_value->bits[5] = 0;
    big_value->bits[6] = small_value.bits[3];
}
void division_by_integer(s21_big_decimal value_1, s21_big_decimal value_2, s21_big_decimal *result)
{
    s21_big_decimal tmp_one_decimal = one_big_decimal;
    s21_big_decimal tmp_result = value_2;
    s21_big_decimal copy_product = big_decimal_null;
    s21_big_decimal copy_result = big_decimal_null;
    s21_big_decimal tmp_value_1 = value_1;
    s21_big_decimal product = big_decimal_null;
    int scale = value_1.bits[6];
    while (mantis_is_less(product, value_1) && mantis_is_greater_or_equal(tmp_value_1, value_2))
    {
        while (mantis_is_less(tmp_result, tmp_value_1))
        {
            shift_left(&tmp_one_decimal, 1);
            multiplication(value_2, tmp_one_decimal, &tmp_result);
        }
        if (mantis_is_greater(tmp_result, tmp_value_1))
        {
            big_shift_right(&tmp_one_decimal, 1);
        }
        big_addition(copy_result, tmp_one_decimal, &copy_result);
        multiplication(tmp_one_decimal, value_2, &copy_product);
        multiplication(copy_result, value_2, &product);
        difference(tmp_value_1, copy_product, &tmp_value_1);
        tmp_result = big_decimal_null;
        tmp_one_decimal = one_big_decimal;
        copy_product = big_decimal_null;
    }
    *result = copy_result;
    result->bits[6] = scale;
}
int mantis_is_less_or_equal(s21_big_decimal value_1, s21_big_decimal value_2)
{
    int result = 1;
    for (int big_bits = 5; big_bits >= 0; big_bits--)
    {
        if (value_1.bits[big_bits] < value_2.bits[big_bits])
        {
            break;
        }
        if (value_1.bits[big_bits] > value_2.bits[big_bits])
        {
            result = 0;
        }
    }
    return result;
}
int mantis_is_less(s21_big_decimal value_1, s21_big_decimal value_2)
{
    int result = 0;
    for (int big_bit = 5; big_bit >= 0; big_bit--)
    {
        if (value_1.bits[big_bit] < value_2.bits[big_bit])
        {
            result = 1;
            break;
        }
        if (value_1.bits[big_bit] > value_2.bits[big_bit])
        {
            break;
        }
    }
    return result;
}
void big_shift_right(s21_big_decimal *value, int step)
{
    for (int i = 0; i < step; i++)
    {
        int count = 0;
        if (big_getBit(*value, 160))
        {
            count = 1;
        }
        value->bits[5] = value->bits[5] >> 1;
        for (int big_bit = 4, number = 1; big_bit >= 0; big_bit--, number++)
        {
            if (count)
            {

                if (big_getBit(*value, 32 * big_bit))
                {

                    count = 1;
                }
                else
                {
                    count = 0;
                }
                value->bits[big_bit] = value->bits[big_bit] >> 1;
                big_setBit(value, 32 * (big_bit + 1) - 1, 1);
            }
            else
            {

                if (big_getBit(*value, 32 * big_bit))
                {
                    count = 1;
                }
                else
                {
                    count = 0;
                }
                value->bits[big_bit] = value->bits[big_bit] >> 1;
            }
        }
    }
}
int shift_right(s21_decimal *value, int step)
{
    int error = 0;
    if (getBit(*value, 0))
    {
        error = 1;
    }
    for (int i = 0; i < step; i++)
    {
        if (!getBit(*value, 64))
        {
            value->bits[2] = value->bits[2] >> 1;
            if (!getBit(*value, 32))
            {
                value->bits[1] = value->bits[1] >> 1;
                value->bits[0] = value->bits[0] >> 1;
            }
            else
            {
                value->bits[1] = value->bits[1] >> 1;
                value->bits[0] = value->bits[0] >> 1;
                setBit(value, 31, 1);
            }
        }
        else
        {
            value->bits[2] = value->bits[2] >> 1;
            if (!getBit(*value, 32))
            {
                value->bits[1] = value->bits[1] >> 1;
                setBit(value, 63, 1);
                value->bits[0] = value->bits[0] >> 1;
            }
            else
            {
                value->bits[1] = value->bits[1] >> 1;
                setBit(value, 63, 1);
                value->bits[0] = value->bits[0] >> 1;
                setBit(value, 31, 1);
            }
        }
    }
    return error;
}
void shift_left(s21_big_decimal *value, int step)
{
    int scale = value->bits[6];
    for (int i = 0; i < step; i++)
    {
        int count = 0;
        if (big_getBit(*value, 31))
        {
            count = 1;
        }
        else
        {
            count = 0;
        }
        value->bits[0] = step_left(value->bits[0]);
        for (int big_bit = 2; big_bit <= 6; big_bit++)
        {
            if (big_getBit(*value, (32 * big_bit - 1)))
            {
                value->bits[big_bit - 1] = step_left(value->bits[big_bit - 1]);
                if (count == 1)
                {
                    big_setBit(value, (32 * (big_bit - 1)), 1);
                }
                count = 1;
            }
            else
            {
                value->bits[big_bit - 1] = step_left(value->bits[big_bit - 1]);
                if (count == 1)
                {
                    big_setBit(value, (32 * (big_bit - 1)), 1);
                }
                count = 0;
            }
        }
    }
    value->bits[6] = scale;
}
int step_left(int bits_decimal)
{
    bits_decimal = bits_decimal << 1;
    return bits_decimal;
}
int multiplication(s21_big_decimal value_1, s21_big_decimal value_2, s21_big_decimal *result)
{
    int error = 0;
    unsigned int count = value_1.bits[6];
    *result = big_decimal_null;
    for (int bit = 0; bit <= 191; bit++)
    {
        if (big_getBit(value_1, bit))
        {
            s21_big_decimal num = value_2;
            shift_left(&num, bit);
            error = big_addition(*result, num, result);
        }
    }
    result->bits[6] = count;
    return error;
}
int mantis_is_greater(s21_big_decimal value_1, s21_big_decimal value_2)
{
    int result = 0;
    for (int big_bit = 5; big_bit >= 0; big_bit--)
    {
        if (value_1.bits[big_bit] > value_2.bits[big_bit])
        {
            result = 1;
            break;
        }
        if (value_1.bits[big_bit] < value_2.bits[big_bit])
        {
            break;
        }
    }
    return result;
}
int mantis_is_greater_or_equal(s21_big_decimal value_1, s21_big_decimal value_2)
{
    int result = 1;
    for (int big_bit = 5; big_bit >= 0; big_bit--)
    {
        if (value_1.bits[big_bit] > value_2.bits[big_bit])
        {
            result = 1;
            break;
        }
        if (value_1.bits[big_bit] < value_2.bits[big_bit])
        {
            result = 0;
            break;
        }
    }
    return result;
}
int mantis_s21_is_equal(s21_big_decimal value_1, s21_big_decimal value_2)
{
    int result = 1;
    for (int big_bit = 0; big_bit <= 5; big_bit++)
    {
        if (value_1.bits[big_bit] != value_2.bits[big_bit])
        {
            result = 0;
        }
    }
    return result;
}
int bank_round(s21_big_decimal value, s21_big_decimal *result)
{
    int error = 0;
    if (check_dop_decimal(value))
    {
        while (!check_five_number(value))
        {

            if (demotion_scale(&value) == 1)
            {
                error = 1;
            }
        }
        if (value.bits[0] == 0)
        {

            if (demotion_scale(&value) == 1)
            {
                error = 1;
            }
            *result = value;
        }

        else
        {
            int current_number = 0;
            if (value.bits[0] / 10 == 0)
            {
                current_number = value.bits[0];
            }
            else
            {
                current_number = value.bits[0] % 10;
            }
            if (check_remainder(current_number))
            {
                if (!parity(value.bits[0]))
                {
                    if (demotion_scale(&value) == 1)
                    {
                        error = 1;
                    }
                    big_addition(value, one_big_decimal, &value);
                    *result = value;
                }
                else
                {

                    if (demotion_scale(&value) == 1)
                    {
                        error = 1;
                    }
                    *result = value;
                }
            }
            else
            {

                if (demotion_scale(&value) == 1)
                {
                    error = 1;
                }
                *result = value;
            }
        }
    }
    return error;
}
int parity(int value)
{
    value = value / 10;
    int result = 0;
    if (value % 2 == 0)
    {
        result = 1;
    }
    return result;
}
int check_remainder(int value)
{
    int result = 0;
    if (value >= 5)
    {
        result = 1;
    }
    return result;
}
int check_dop_decimal(s21_big_decimal value)
{
    int result = 0;
    for (int big_bits = 5; big_bits >= 3; big_bits--)
    {
        if (value.bits[big_bits] != 0)
        {
            result = 1;
        }
    }
    return result;
}
int check_five_number(s21_big_decimal value)
{
    int result = 1;
    for (int big_bits = 5; big_bits >= 4; big_bits--)
    {
        if (value.bits[big_bits] != 0)
        {
            result = 0;
        }
    }
    if (result && value.bits[3] / 10 < 10)
    {
    }
    else
    {
        result = 0;
    }
    return result;
}