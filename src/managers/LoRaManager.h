#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// LoRa pin definitions
#define LORA_SS_PIN 5
#define LORA_RST_PIN 14
#define LORA_DIO0_PIN 27

// LoRa frequency for Asia
#define LORA_FREQUENCY 433E6
#define LORA_SYNC_WORD 0xF3

// LoRa state structure
struct LoRaState {
    bool initialized = false;
    bool available = false;
    int rssi = 0;
    float snr = 0.0;
    String lastMessage = "";
    unsigned long lastMessageTime = 0;
};

extern LoRaState loraState;

// LoRa functions
void initializeLoRa();
bool sendLoRaMessage(String message);
void checkLoRaMessages();
void handleLoRaMessage(String message);
bool isLoRaAvailable();
int getLoRaRSSI();