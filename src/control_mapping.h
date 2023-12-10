#pragma once
#ifndef CONTROL_MAPPING_H
#define CONTROL_MAPPING_H

#include "easy_espnow.h"
#include "elapsedMillis.h"
#include "helper.h"

typedef struct struct_control
{
    float steer = 0;
    float speed = 0;
    int range = 200;
} struct_control;
struct_control control;

float acc_x_max = 1;
float acc_y_max = 1;
short int acc_y_offset = 90;
int acc_y;

elapsedMillis since_acc_control = 0;

typedef struct
{
    uint16_t activated;
    int16_t max_x;
    int16_t min_x;
} FastMode;
FastMode fastMode;

message_from_remote map_control(int analog_x, int analog_y, int analog_z)
{
    message_from_remote msg;
    #define MAX_VALUE 4095.0

    float x = clipf(((float(analog_x) / MAX_VALUE) - 0.5) * -2, -1, 1);
    float y = clipf(((float(analog_y) / MAX_VALUE) - 0.5) * -2, -1, 1);
    float z = clipf(float(analog_z) / MAX_VALUE, 0, 1);
    z = 1 - z;

    control.steer = x;
    control.speed = y;
    control.range = clipf(z * 8, 0, 1) * 1000;

#if DEBUG_MAPPING
    Serial.printf("x: %2.2f y: %2.2f z: %2.2f | ", x, y, z);
    // Serial.printf("steer: %2.2f speed: %2.2f, range: %4d | " control.steer, control.speed, control.range);
#endif
    // ========================= convert cotrol to msg =========================

#define STEER_FACTOR 1
#if USE_DUAL_BOARDS
        float speed_Left = control.speed + control.steer * STEER_FACTOR * (-1);
        speed_Left = clipf(speed_Left, -1, 1) * control.range;
        msg.cmd_Left_L = speed_Left;
        msg.cmd_Left_R = speed_Left;
        
        float speed_Right = control.speed - control.steer * STEER_FACTOR * (-1);
        speed_Right = clipf(speed_Right, -1, 1) * control.range;

        msg.cmd_Right_L = speed_Right;
        msg.cmd_Right_R = speed_Right;

#else
        msg.cmd_L = (int16_t)((control.speed + control.steer * STEER_FACTOR) * control.range);
        msg.cmd_R = (int16_t)((control.speed - control.steer * STEER_FACTOR) * control.range);

        msg.cmd_L = clipf(msg.cmd_L, -control.range, control.range);
        msg.cmd_R = clipf(msg.cmd_R, -control.range, control.range);
#endif

    return msg;
}

#endif
