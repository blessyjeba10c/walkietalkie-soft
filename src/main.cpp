
#include <Arduino.h>
#include "WalkieTalkie.h"
#include "managers/GPSManager.h"
#include "managers/GSMManager.h"
#include "managers/LoRaManager.h"
#include "CommandProcessor.h"
#include "managers/DisplayManager.h"
#include "managers/KeyboardManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// Task handles (optional, for control)
TaskHandle_t keyboardTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t gpsTaskHandle = NULL;
TaskHandle_t loraTaskHandle = NULL;
TaskHandle_t gsmTaskHandle = NULL;
TaskHandle_t dmrTaskHandle = NULL;
TaskHandle_t btTaskHandle = NULL;

// Task intervals (ms)
#define KEYBOARD_TASK_INTERVAL    10
#define DISPLAY_TASK_INTERVAL     50
#define GPS_TASK_INTERVAL         200
#define LORA_TASK_INTERVAL        30
#define GSM_TASK_INTERVAL         5000
#define DMR_TASK_INTERVAL         5
#define BT_TASK_INTERVAL          10
#define BT_KEEPALIVE_INTERVAL     15000

void keyboardTask(void *pvParameters) {
    while (1) {
        scanKeyboard();
        vTaskDelay(pdMS_TO_TICKS(KEYBOARD_TASK_INTERVAL));
    }
}

void displayTask(void *pvParameters) {
    while (1) {
        updateDisplay();
        vTaskDelay(pdMS_TO_TICKS(DISPLAY_TASK_INTERVAL));
    }
}

void gpsTask(void *pvParameters) {
    while (1) {
        readGPS();
        handleContinuousGPS();
        vTaskDelay(pdMS_TO_TICKS(GPS_TASK_INTERVAL));
    }
}

void loraTask(void *pvParameters) {
    while (1) {
        checkLoRaMessages();
        vTaskDelay(pdMS_TO_TICKS(LORA_TASK_INTERVAL));
    }
}

void gsmTask(void *pvParameters) {
    while (1) {
        checkIncomingGSMSMS();
        vTaskDelay(pdMS_TO_TICKS(GSM_TASK_INTERVAL));
    }
}

void dmrTask(void *pvParameters) {
    while (1) {
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
        vTaskDelay(pdMS_TO_TICKS(DMR_TASK_INTERVAL));
    }
}

void btTask(void *pvParameters) {
    static unsigned long lastBTKeepAlive = 0;
    static uint32_t lastHeap = 0;
    while (1) {
        handleBluetoothCommands();
        unsigned long now = millis();
        if (now - lastBTKeepAlive >= BT_KEEPALIVE_INTERVAL) {
            lastBTKeepAlive = now;
            if (SerialBT.hasClient()) {
                SerialBT.write(0); // Send null byte as heartbeat
            }
            uint32_t freeHeap = ESP.getFreeHeap();
            if (lastHeap > 0 && freeHeap < lastHeap - 5000) {
                SerialBT.print("⚠️ Heap dropped: ");
                SerialBT.print(lastHeap);
                SerialBT.print(" -> ");
                SerialBT.println(freeHeap);
            }
            lastHeap = freeHeap;
        }
        vTaskDelay(pdMS_TO_TICKS(BT_TASK_INTERVAL));
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
    
    SerialBT.println("🚀 System initialized (RTOS Mode)");
    SerialBT.println("✅ Bluetooth should be stable now!");

    // Create tasks with priorities (higher number = higher priority)
    xTaskCreatePinnedToCore(keyboardTask, "KeyboardTask", 2048, NULL, 3, &keyboardTaskHandle, 1);
    xTaskCreatePinnedToCore(displayTask,  "DisplayTask",  2048, NULL, 2, &displayTaskHandle, 1);
    xTaskCreatePinnedToCore(gpsTask,      "GPSTask",      2048, NULL, 1, &gpsTaskHandle, 1);
    xTaskCreatePinnedToCore(loraTask,     "LoRaTask",     2048, NULL, 2, &loraTaskHandle, 1);
    xTaskCreatePinnedToCore(gsmTask,      "GSMTask",      2048, NULL, 1, &gsmTaskHandle, 1);
    xTaskCreatePinnedToCore(dmrTask,      "DMRTask",      4096, NULL, 2, &dmrTaskHandle, 1);
    xTaskCreatePinnedToCore(btTask,       "BTTask",       2048, NULL, 2, &btTaskHandle, 1);
}


void loop() {
    // Empty: all logic is handled by RTOS tasks
    vTaskDelete(NULL);
}