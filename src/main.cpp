#include <Arduino.h>
#include "WalkieTalkie.h"
#include "managers/GPSManager.h"
#include "managers/GSMManager.h"
#include "managers/LoRaManager.h"
#include "CommandProcessor.h"
#include "managers/DisplayManager.h"
#include "managers/KeyboardManager.h"

// FreeRTOS Task Handles
TaskHandle_t dmrTaskHandle = NULL;
TaskHandle_t gpsTaskHandle = NULL;
TaskHandle_t gsmTaskHandle = NULL;
TaskHandle_t loraTaskHandle = NULL;
TaskHandle_t bluetoothTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t keyboardTaskHandle = NULL;

// Task priorities (0 = lowest, 25 = highest)
#define DMR_TASK_PRIORITY       5   // High priority for real-time radio
#define GPS_TASK_PRIORITY       3   // Medium priority for location
#define GSM_TASK_PRIORITY       3   // Medium priority for SMS
#define LORA_TASK_PRIORITY      4   // Medium-high for mesh networking
#define BLUETOOTH_TASK_PRIORITY 4   // Medium-high for user interface
#define DISPLAY_TASK_PRIORITY   2   // Lower priority for display updates
#define KEYBOARD_TASK_PRIORITY  3   // Medium priority for input

// Stack sizes (in words, 4 bytes each)
#define DMR_STACK_SIZE         4096
#define GPS_STACK_SIZE         2048
#define GSM_STACK_SIZE         3072
#define LORA_STACK_SIZE        2048
#define BLUETOOTH_STACK_SIZE   4096  // Increased for stability
#define DISPLAY_STACK_SIZE     2048
#define KEYBOARD_STACK_SIZE    1024

// FreeRTOS Task Functions
void dmrTask(void *parameter) {
    while (true) {
        // Handle DMR radio events - highest priority
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
        
        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay
    }
}

void gpsTask(void *parameter) {
    while (true) {
        // Handle GPS data reading and processing
        readGPS();
        handleContinuousGPS();
        
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay - GPS doesn't need high frequency
    }
}

void gsmTask(void *parameter) {
    while (true) {
        // Check for incoming GSM SMS (queue-based)
        checkIncomingGSMSMS();
        
        vTaskDelay(pdMS_TO_TICKS(3000)); // 3 seconds delay - Queue-based SMS checking
    }
}

void loraTask(void *parameter) {
    while (true) {
        // Check for incoming LoRa messages
        checkLoRaMessages();
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms delay - faster for mesh networking
    }
}

void bluetoothTask(void *parameter) {
    static unsigned long lastKeepAlive = 0;
    static bool wasConnected = false;
    
    while (true) {
        // Send keep-alive every 30 seconds to prevent timeout
        unsigned long currentTime = millis();
        if (currentTime - lastKeepAlive >= 30000) {
            lastKeepAlive = currentTime;
            if (SerialBT.hasClient()) {
                SerialBT.print(""); // Empty write to keep connection alive
            }
        }
        
        // Check connection status
        bool isConnected = SerialBT.hasClient();
        if (isConnected && !wasConnected) {
            SerialBT.println("\n✅ Bluetooth client connected!");
            wasConnected = true;
        } else if (!isConnected && wasConnected) {
            wasConnected = false;
        }
        
        // Handle Bluetooth commands and communication
        handleBluetoothCommands();
        
        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay - more responsive
    }
}

void displayTask(void *parameter) {
    while (true) {
        // Update display - lowest priority, can be delayed
        updateDisplay();
        
        vTaskDelay(pdMS_TO_TICKS(200)); // 200ms delay - display updates don't need to be frequent
    }
}

void keyboardTask(void *parameter) {
    while (true) {
        // Handle keyboard input
        scanKeyboard();
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms delay - responsive input
    }
}

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
    
    SerialBT.println("🚀 Creating FreeRTOS tasks...");
    
    // Create FreeRTOS tasks
    xTaskCreatePinnedToCore(
        dmrTask,           // Task function
        "DMR_Task",        // Task name
        DMR_STACK_SIZE,    // Stack size
        NULL,              // Parameters
        DMR_TASK_PRIORITY, // Priority
        &dmrTaskHandle,    // Task handle
        0                  // CPU core (0 or 1)
    );
    
    xTaskCreatePinnedToCore(
        gpsTask,
        "GPS_Task",
        GPS_STACK_SIZE,
        NULL,
        GPS_TASK_PRIORITY,
        &gpsTaskHandle,
        1  // Use core 1 for GPS
    );
    
    xTaskCreatePinnedToCore(
        gsmTask,
        "GSM_Task",
        GSM_STACK_SIZE,
        NULL,
        GSM_TASK_PRIORITY,
        &gsmTaskHandle,
        1  // Use core 1 for GSM
    );
    
    xTaskCreatePinnedToCore(
        loraTask,
        "LoRa_Task",
        LORA_STACK_SIZE,
        NULL,
        LORA_TASK_PRIORITY,
        &loraTaskHandle,
        0  // Use core 0 for LoRa
    );
    
    xTaskCreatePinnedToCore(
        bluetoothTask,
        "BT_Task",
        BLUETOOTH_STACK_SIZE,
        NULL,
        BLUETOOTH_TASK_PRIORITY,
        &bluetoothTaskHandle,
        0  // Use core 0 for Bluetooth
    );
    
    xTaskCreatePinnedToCore(
        displayTask,
        "Display_Task",
        DISPLAY_STACK_SIZE,
        NULL,
        DISPLAY_TASK_PRIORITY,
        &displayTaskHandle,
        1  // Use core 1 for display
    );
    
    xTaskCreatePinnedToCore(
        keyboardTask,
        "Keyboard_Task",
        KEYBOARD_STACK_SIZE,
        NULL,
        KEYBOARD_TASK_PRIORITY,
        &keyboardTaskHandle,
        1  // Use core 1 for keyboard
    );
    
    SerialBT.println("✅ All FreeRTOS tasks created successfully!");
    SerialBT.println("📊 Task Distribution:");
    SerialBT.println("   Core 0: DMR, LoRa, Bluetooth");
    SerialBT.println("   Core 1: GPS, GSM, Display, Keyboard");
}

void loop() {
    // FreeRTOS scheduler handles everything
    // Keep loop alive to feed watchdog and maintain Bluetooth
    vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second delay
}