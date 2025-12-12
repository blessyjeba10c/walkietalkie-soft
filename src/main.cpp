#include <Arduino.h>
#include "WalkieTalkie.h"
#include "managers/GPSManager.h"
#include "managers/GSMManager.h"
#include "managers/LoRaManager.h"
#include "CommandProcessor.h"
#include "managers/DisplayManager.h"
#include "managers/KeyboardManager.h"

// Timing variables for non-blocking delays
unsigned long lastDMRUpdate = 0;
unsigned long lastGPSUpdate = 0;
unsigned long lastGSMCheck = 0;
unsigned long lastLoRaCheck = 0;
unsigned long lastBluetoothUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastKeyboardScan = 0;
unsigned long lastBTKeepAlive = 0;

// Update intervals (milliseconds)
#define DMR_UPDATE_INTERVAL       5     // 5ms - high frequency for radio
#define GPS_UPDATE_INTERVAL       200   // 200ms - GPS parsing (reduced frequency)
#define GSM_CHECK_INTERVAL        5000  // 5 seconds - SMS queue check (reduced frequency)
#define LORA_CHECK_INTERVAL       30    // 30ms - LoRa message check
#define BLUETOOTH_UPDATE_INTERVAL 0     // 0ms - always check (highest priority)
#define DISPLAY_UPDATE_INTERVAL   300   // 300ms - display refresh
#define KEYBOARD_SCAN_INTERVAL    20    // 20ms - keyboard input (more responsive)
#define BT_KEEPALIVE_INTERVAL     15000 // 15 seconds - heartbeat

void setup() {
    // Initialize all system components
    initializeSystem();
    
    // Setup based on current mode
    switch (currentMode) {
        case MODE_BASIC_TEST:
            setupBasicTest();
            break;
        case MODE_WALKIE_FEATURES:
            setupWalkieFeatures();
            break;
        case MODE_LOW_LEVEL:
            setupLowLevel();
            break;
    }
    
    // Show available commands
    showCommands();
    
    SerialBT.println("🚀 System initialized (No RTOS - Sequential Loop)");
    SerialBT.println("✅ Bluetooth should be stable now!");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Bluetooth Update - HIGHEST PRIORITY (always check)
    handleBluetoothCommands();
    
    // Keyboard Scan - HIGH PRIORITY (20ms)
    if (currentTime - lastKeyboardScan >= KEYBOARD_SCAN_INTERVAL) {
        lastKeyboardScan = currentTime;
        scanKeyboard();
    }
    
    // DMR Radio Update (5ms)
    if (currentTime - lastDMRUpdate >= DMR_UPDATE_INTERVAL) {
        lastDMRUpdate = currentTime;
        dmr.update();
        
        // Run mode-specific DMR handling
        switch (currentMode) {
            case MODE_BASIC_TEST:
                loopBasicTest();
                break;
            case MODE_WALKIE_FEATURES:
                loopWalkieFeatures();
                break;
            case MODE_LOW_LEVEL:
                loopLowLevel();
                break;
        }
    }
    
    // LoRa Message Check (30ms)
    if (currentTime - lastLoRaCheck >= LORA_CHECK_INTERVAL) {
        lastLoRaCheck = currentTime;
        checkLoRaMessages();
    }
    
    // GPS Update (200ms)
    if (currentTime - lastGPSUpdate >= GPS_UPDATE_INTERVAL) {
        lastGPSUpdate = currentTime;
        readGPS();
        handleContinuousGPS();
    }
    
    // Display Update (300ms)
    if (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        lastDisplayUpdate = currentTime;
        updateDisplay();
    }
    
    // GSM SMS Check (5 seconds - lowest priority)
    if (currentTime - lastGSMCheck >= GSM_CHECK_INTERVAL) {
        lastGSMCheck = currentTime;
        checkIncomingGSMSMS();
    }
    
    // Bluetooth Keep-Alive (15 seconds)
    if (currentTime - lastBTKeepAlive >= BT_KEEPALIVE_INTERVAL) {
        lastBTKeepAlive = currentTime;
        if (SerialBT.hasClient()) {
            SerialBT.write(0); // Send null byte as heartbeat
        }
        // Monitor heap to detect memory leaks
        static uint32_t lastHeap = 0;
        uint32_t freeHeap = ESP.getFreeHeap();
        if (lastHeap > 0 && freeHeap < lastHeap - 5000) {
            Serial.print("⚠️ Heap dropped: ");
            Serial.print(lastHeap);
            Serial.print(" -> ");
            Serial.println(freeHeap);
        }
        lastHeap = freeHeap;
    }
    
    // Yield to prevent task watchdog issues
    yield();
}