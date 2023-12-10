#pragma once
#ifndef ESPNOW_PROTOCOL_H
#define ESPNOW_PROTOCOL_H

#if ESP8266
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif ESP32  // https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/
#include <esp_now.h>
#include <WiFi.h>
#endif

#include "config.h"

// ========================= PROTOCOL =========================
typedef struct message_from_remote
{
#if USE_CMD_L_R
#if USE_DUAL_BOARDS
    int16_t cmd_Left_L=0;
    int16_t cmd_Left_R=0;;
    int16_t cmd_Right_L=0;
    int16_t cmd_Right_R=0;
#else
    int16_t cmd_L;
    int16_t cmd_R;
#endif
#endif
} message_from_remote;

    message_from_remote msg_from_remote;

typedef struct message_to_remote
{
#if USE_CMD_L_R
#if USE_DUAL_BOARDS
    int16_t speed_Left_L;
    int16_t speed_Left_R;
    int16_t speed_Right_L;
    int16_t speed_Right_R;
#else
    int16_t speed_L;
    int16_t speed_R;
#endif
#endif
} message_to_remote;

    message_to_remote msg_from_receiver;

uint8_t *address_target = nullptr;


//================================================================
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
#if 0
    if (sendStatus == 0)
        Serial.print("Delivery success  - ");
    else
        Serial.print("Delivery fail     - ");
#endif
}

typedef void (*ESPNOW_RX_data_callback)(uint8_t *data, uint8_t len);
ESPNOW_RX_data_callback receiveBytes = nullptr;

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
    if (receiveBytes != nullptr)
        receiveBytes(incomingData, len);
}

void ESPNOW_Init(ESPNOW_RX_data_callback callback)
{
    receiveBytes = callback;

#if IS_REMOTE
    address_target = MAC_ADDRESS_RECEIVER;
#else
    address_target = MAC_ADDRESS_REMOTE;
#endif

#if 1
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
#endif

    if (esp_now_init() != 0)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Set ESP-NOW Role
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    esp_now_add_peer(address_target, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);
}

void ESPNOW_sendBytes(uint8_t *data, uint8_t len)
{
    if (address_target != nullptr)
        esp_now_send(address_target, data, len);
}


void ESPNOW_sendMessage(message_to_remote *msg)
{
    if (address_target != nullptr)
    {
        message_to_remote message;
        memcpy(&message, msg, sizeof(message_to_remote));

        uint8_t *data = (uint8_t *)&message;
        ESPNOW_sendBytes(data, sizeof(message_to_remote));
    }
}

void ESPNOW_sendMessage(message_from_remote *msg)
{
    if (address_target != nullptr)
    {
        message_from_remote message;
        memcpy(&message, msg, sizeof(message_from_remote));

        message.cmd_Left_L = INVERT_CMD_LEFT_L ? -message.cmd_Left_L : message.cmd_Left_L;
        message.cmd_Left_R = INVERT_CMD_LEFT_R ? -message.cmd_Left_R : message.cmd_Left_R;
        message.cmd_Right_L = INVERT_CMD_RIGHT_L ? -message.cmd_Right_L : message.cmd_Right_L;
        message.cmd_Right_R = INVERT_CMD_RIGHT_R ? -message.cmd_Right_R : message.cmd_Right_R;

        uint8_t *data = (uint8_t *)&message;
        ESPNOW_sendBytes(data, sizeof(message_from_remote));
    }
}

#endif // ESPNOW_PROTOCOL_H