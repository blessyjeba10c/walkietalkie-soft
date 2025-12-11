#include "LoRaManager.h"
#include "WalkieTalkie.h"
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;

LoRaState loraState;

void initializeLoRa() {
    SerialBT.println("📡 Initializing LoRa module...");
    
    // Set LoRa pins
    LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
    
    // Initialize LoRa with Asian frequency
    int attempts = 0;
    while (!LoRa.begin(LORA_FREQUENCY) && attempts < 10) {
        SerialBT.print(".");
        delay(500);
        attempts++;
    }
    
    if (attempts < 10) {
        // LoRa initialized successfully
        LoRa.setSyncWord(LORA_SYNC_WORD);
        LoRa.setTxPower(20); // Set max transmission power
        LoRa.setSpreadingFactor(7); // Balance between range and speed
        LoRa.setSignalBandwidth(125E3); // Standard bandwidth
        LoRa.setCodingRate4(5); // Error correction
        
        loraState.initialized = true;
        loraState.available = true;
        
        SerialBT.println("✅ LoRa module initialized");
        SerialBT.println("📡 LoRa Config:");
        SerialBT.println("  Frequency: 433MHz");
        SerialBT.println("  Sync Word: 0xF3");
        SerialBT.println("  TX Power: 20dBm");
    } else {
        SerialBT.println("❌ LoRa module not responding");
        loraState.initialized = false;
        loraState.available = false;
    }
}

bool sendLoRaMessage(String message) {
    if (!loraState.initialized || !loraState.available) {
        SerialBT.println("❌ LoRa not ready for transmission");
        return false;
    }
    
    SerialBT.println("📡 Sending LoRa message...");
    SerialBT.println("Message: " + message);
    
    // Send LoRa packet
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    
    SerialBT.println("✅ LoRa message sent");
    return true;
}

void checkLoRaMessages() {
    if (!loraState.initialized) return;
    
    // Check for incoming LoRa packets
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        // Read packet
        String message = "";
        while (LoRa.available()) {
            message += (char)LoRa.read();
        }
        
        // Get signal quality
        loraState.rssi = LoRa.packetRssi();
        loraState.snr = LoRa.packetSnr();
        loraState.lastMessage = message;
        loraState.lastMessageTime = millis();
        
        SerialBT.println("\n📡 LoRa Message Received:");
        SerialBT.println("Message: " + message);
        SerialBT.println("RSSI: " + String(loraState.rssi) + " dBm");
        SerialBT.println("SNR: " + String(loraState.snr) + " dB");
        SerialBT.println("Packet Size: " + String(packetSize));
        
        // Handle the received message
        handleLoRaMessage(message);
    }
}

void handleLoRaMessage(String message) {
    // Check if this is a GPS message and parse it
    if (message.startsWith("GPS ")) {
        // Parse GPS coordinates from LoRa
        extern void parseIncomingGPS(String message, String commMode);
        parseIncomingGPS(message, "LoRa");
    } else if (message.startsWith("EMERGENCY:")) {
        // Handle emergency message
        SerialBT.println("🚨 Emergency alert received via LoRa!");
        SerialBT.println("Details: " + message.substring(10));
    } else {
        // Regular message
        SerialBT.println("💬 LoRa Text: " + message);
    }
}

bool isLoRaAvailable() {
    return loraState.initialized && loraState.available;
}

int getLoRaRSSI() {
    return loraState.rssi;
}