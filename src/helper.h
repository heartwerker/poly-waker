
#ifndef HELPER_H
#define HELPER_H
#include <Arduino.h>

float clipf(float value, float min, float max)
{
    value = value > max ? max : value;
    value = value < min ? min : value;
    return value;
}

float convert_zero_zone(float value, float zero_zone)
{
    float sign = value > 0 ? 1 : -1;

    value = fabs(value);
    value -= zero_zone;
    value /= (1.0 - zero_zone);

    value = clipf(value, 0, 1);

    return value * sign;
}


#endif // HELPER_H#