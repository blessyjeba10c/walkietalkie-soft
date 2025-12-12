#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <LoRaReliable.h>

// LoRa pin definitions
#define LORA_SS_PIN 5
#define LORA_RST_PIN 14
#define LORA_DIO0_PIN 27

// LoRa frequency for Asia
#define LORA_FREQUENCY 433E6

// LoRa state structure
struct LoRaState {
    bool initialized = false;
    bool available = false;
    int rssi = 0;
    float snr = 0.0;
    String lastMessage = "";
    unsigned long lastMessageTime = 0;
    
    // ACK mode (now handled by LoRaReliable)
    bool ackMode = true; // Default to ACK mode enabled
    int maxRetries = 3;
    unsigned long ackTimeout = 2000; // 2 seconds
};

extern LoRaState loraState;
extern LoRaReliable loraReliable;

// LoRa functions
void initializeLoRa();
bool sendLoRaMessage(String message);
void checkLoRaMessages();
void handleLoRaMessage(String message);
bool isLoRaAvailable();
int getLoRaRSSI();
float getLoRaSNR();
void enableLoRaAck();
void disableLoRaAck();