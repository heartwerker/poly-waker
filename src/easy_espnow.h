#ifndef EASY_ESPNOW_H
#define EASY_ESPNOW_H

#include <Arduino.h>
#include "config.h"

#if ESP8266
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif ESP32  // https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/
#include <esp_now.h>
#include <WiFi.h>
esp_now_peer_info_t peerInfo;
#endif

// ========================= PROTOCOL =========================
typedef struct message_generic
{
    uint8_t index = 0;
    float value = 0;
} message_generic;

message_generic msg;
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
//================================================================

typedef void (*esp_now_send_cb_t)(const uint8_t *mac_addr, esp_now_send_status_t status);
//================================================================
#if ESP8266
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
#if 0
    if (sendStatus == 0)
        Serial.print("Delivery success  - ");
    else
        Serial.print("Delivery fail     - ");
#endif
}
#elif ESP32
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
#if 0
    Serial.print("\rLast Packet Send:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
#endif
}
#endif

typedef void (*ESPNOW_RX_data_callback)(uint8_t *data, uint8_t len);
ESPNOW_RX_data_callback receiveBytes = nullptr;

#if ESP8266
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
    if (receiveBytes != nullptr)
        receiveBytes(incomingData, len);
}
#elif ESP32
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
    if (receiveBytes != nullptr)
        receiveBytes((uint8_t *)incomingData, len);
}
#endif

void ESPNOW_registerReceiver(unsigned char *address)
{
    // Register peer
    memcpy(peerInfo.peer_addr, address, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }
}

void ESPNOW_Init(ESPNOW_RX_data_callback callback, unsigned char *address_target)
{
    receiveBytes = callback;

#if 1 // todo is this necessary ?
    // Set device as a Wi-Fi Station
    // WiFi.mode(WIFI_STA);
    WiFi.mode(WIFI_AP_STA);
    //WiFi.disconnect();
#endif

    if (esp_now_init() != 0)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

#if ESP8266
    // Set ESP-NOW Role
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
#endif

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register peer
#if ESP8266
    esp_now_add_peer(address_target, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
#elif ESP32
    ESPNOW_registerReceiver(address_target);
#endif
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);

    Serial.println("ESPNOW_Init() done");
}


#endif // EASY_ESPNOW_H