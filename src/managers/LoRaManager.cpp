#include "LoRaManager.h"
#include "WalkieTalkie.h"
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;

LoRaState loraState;
LoRaReliable loraReliable(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

void initializeLoRa() {
    SerialBT.println("📡 Initializing LoRa module...");
    
    // Set debug output to Bluetooth
    loraReliable.setDebugOutput(&SerialBT);
    
    // Initialize LoRa with Asian frequency
    if (loraReliable.begin(LORA_FREQUENCY)) {
        loraState.initialized = true;
        loraState.available = true;
        
        SerialBT.println("✅ LoRa module initialized");
        SerialBT.println("📡 LoRa Config:");
        SerialBT.println("  Frequency: 433MHz");
        SerialBT.println("  TX Power: 20dBm");
        SerialBT.println("  Spreading Factor: 7");
        SerialBT.println("  Bandwidth: 125kHz");
        SerialBT.println("  Coding Rate: 4/5");
        SerialBT.println("  CRC: Enabled");
        SerialBT.println("  ACK Mode: " + String(loraState.ackMode ? "ENABLED" : "DISABLED"));
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
    
    // Prepend network ID for filtering
    extern WalkieTalkieState wtState;
    String fullMessage = "[" + wtState.loraNetworkID + "]" + message;
    
    SerialBT.println("📡 Sending LoRa message...");
    SerialBT.println("Network ID: " + wtState.loraNetworkID);
    SerialBT.println("Message: " + message);
    
    bool success;
    if (loraState.ackMode) {
        // Send with ACK and retry
        success = loraReliable.send(fullMessage, loraState.maxRetries, loraState.ackTimeout);
    } else {
        // Broadcast mode - no ACK (single retry)
        success = loraReliable.send(fullMessage, 1, 0);
    }
    
    if (success) {
        SerialBT.println("✅ LoRa message sent successfully");
    } else {
        SerialBT.println("❌ LoRa message failed after retries");
    }
    
    return success;
}

void checkLoRaMessages() {
    if (!loraState.initialized) return;
    
    // Check for incoming LoRa packets
    String message = loraReliable.receive();
    
    if (message.length() > 0) {
        extern WalkieTalkieState wtState;
        
        // Check if message starts with network ID
        String expectedPrefix = "[" + wtState.loraNetworkID + "]";
        
        if (message.startsWith(expectedPrefix)) {
            // Remove network ID prefix
            String actualMessage = message.substring(expectedPrefix.length());
            
            // Get signal quality
            loraState.rssi = loraReliable.getRSSI();
            loraState.snr = loraReliable.getSNR();
            loraState.lastMessage = actualMessage;
            loraState.lastMessageTime = millis();
            
            SerialBT.println("\n📡 LoRa Message Received:");
            SerialBT.println("Network ID: " + wtState.loraNetworkID + " ✅");
            SerialBT.println("Message: " + actualMessage);
            SerialBT.println("RSSI: " + String(loraState.rssi) + " dBm");
            SerialBT.println("SNR: " + String(loraState.snr) + " dB");
            
            // Handle the received message
            handleLoRaMessage(actualMessage);
        } else {
            // Wrong network ID - ignore message
            SerialBT.println("\n📡 LoRa Message Ignored (wrong network ID)");
            SerialBT.println("Expected: " + expectedPrefix);
            SerialBT.println("Received prefix: " + message.substring(0, min(10, (int)message.length())));
        }
    }
}

void handleLoRaMessage(String message) {
    // Add to message history
    extern void addMessage(String message);
    String historyEntry = "LoRa:" + message;
    addMessage(historyEntry);
    
    // Add to message overlay queue for display
    extern void addMessageToQueue(String message);
    String displayMsg = "[LoRa]\n" + message;
    addMessageToQueue(displayMsg);
    
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

float getLoRaSNR() {
    return loraState.snr;
}

void enableLoRaAck() {
    loraState.ackMode = true;
    SerialBT.println("✅ LoRa ACK mode enabled");
    SerialBT.println("   Retries: " + String(loraState.maxRetries));
    SerialBT.println("   Timeout: " + String(loraState.ackTimeout) + " ms");
}

void disableLoRaAck() {
    loraState.ackMode = false;
    SerialBT.println("✅ LoRa ACK mode disabled (broadcast only)");
}