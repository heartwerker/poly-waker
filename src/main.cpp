#include <Arduino.h>

#include "config.h"

#include "easy_espnow.h"
void ESPNOW_receiveBytes(uint8_t *data, uint8_t len);

#include "elapsedMillis.h"

#include "control_mapping.h"

// ================================================================================================

void setup()
{
    Serial.begin(115200);

    ESPNOW_Init(ESPNOW_receiveBytes, MAC_ADDRESS_RECEIVER);

    analogReadResolution(12); // 12 bits

    Serial.println("Starting loop() ...");
}

// ================================================================================================
elapsedMillis sinceLastRead = 0;
void loop()
{
    if (sinceLastRead > 50)
    {
        int x, y, z;
        y = analogRead(35);
        x = analogRead(39);
        z = analogRead(34);

#if DEBUG_RAW
        Serial.printf("ADCs: %d, %d, %d ", x, y, z);
#endif

#if USE_ESPNOW
        message_from_remote msg = map_control(x, y, z);

#if DEBUG_TX
#if DEBUG_FOR_PLOTTER
        // Serial.printf("%4d,%4d,%4d,%4d,", msg.cmd_Left_L, msg.cmd_Left_R, msg.cmd_Right_L, msg.cmd_Right_R);
#else
#if USE_DUAL_BOARDS
        Serial.printf("CMDs Left_LR: %4d,%4d, Right_LR: %4d,%4d  |", msg.cmd_Left_L, msg.cmd_Left_R, msg.cmd_Right_L, msg.cmd_Right_R);
#else
        Serial.printf("CMDs L: %4d, R: %4d  |", msg.cmd_L, msg.cmd_R);
#endif
#endif
#endif
        Serial.println();

#if 1
        esp_now_send(MAC_ADDRESS_RECEIVER, (uint8_t *)&msg, sizeof(msg));
#endif

#endif
    }

    delay(1);
}

// ########################## RX callback ##########################
void ESPNOW_receiveBytes(uint8_t *data, uint8_t len)
{
    memcpy(&msg, data, len);
    Serial.printf("ESPNOW_receiveBytes: %d - %f\n", msg.index, msg.value);
}
