#include <Arduino.h>
#include "WalkieTalkie.h"
#include "managers/GPSManager.h"
#include "managers/GSMManager.h"
#include "managers/LoRaManager.h"
#include "CommandProcessor.h"
#include "managers/DisplayManager.h"
#include "managers/KeyboardManager.h"

// External state variables
extern GPSState gpsState;
extern LoRaState loraState;
extern GSMState gsmState;
extern LoRaReliable loraReliable;
extern DisplayState displayState;

// Message sending with fallback
bool sendMessageWithFallback(String message);

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

// Send message with fallback (DMR -> LoRa -> GSM)
bool sendMessageWithFallback(String message) {
    bool sent = false;
    
    SerialBT.println("📤 Sending message with fallback...");
    
    // Try DMR first (fastest, most reliable for tactical)
    SerialBT.println("📡 Attempting DMR transmission...");
    if (dmr.sendSMS(0xFFFFFF, message.c_str())) { // Broadcast to all
        SerialBT.println("✅ Message sent via DMR");
        sent = true;
        return sent;
    }
    
    // Fallback to LoRa if DMR fails
    if (!sent && loraState.initialized) {
        SerialBT.println("📡 DMR failed, trying LoRa...");
        if (sendLoRaMessage(message)) {
            SerialBT.println("✅ Message sent via LoRa");
            sent = true;
            return sent;
        }
    }
    
    // Fallback to GSM if LoRa fails
    if (!sent && gsmState.initialized && gsmState.networkRegistered) {
        SerialBT.println("📡 LoRa failed, trying GSM...");
        String phoneNumber = "+1234567890"; // TODO: Make configurable
        sendGSMFallbackSMS(phoneNumber, message);
        SerialBT.println("✅ Message sent via GSM");
        sent = true;
        return sent;
    }
    
    if (!sent) {
        SerialBT.println("❌ All transmission methods failed");
    }
    
    return sent;
}

// Tracker mode task - GPS reading and transmission with fallback
void trackerTask(void *parameter) {
    static unsigned long lastTransmit = 0;
    static unsigned long lastGSMCheck = 0;
    const unsigned long TRANSMIT_INTERVAL = 30000; // Send every 30 seconds
    const unsigned long GSM_CHECK_INTERVAL = 5000; // Check GSM every 5 seconds
    
    while (true) {
        readGPS();
        handleContinuousGPS();
        
        // Check for incoming messages from all channels
        dmr.update(); // Handles DMR messages
        
        // Check LoRa messages
        if (loraState.initialized) {
            String loraMsg = loraReliable.receive();
            if (loraMsg.length() > 0 && !loraMsg.startsWith("GPS")) {
                // Non-GPS message - add to display queue
                addMessageToQueue("[LoRa] " + loraMsg);
                SerialBT.println("📩 Message received via LoRa: " + loraMsg);
            }
        }
        
        // Check GSM messages periodically
        unsigned long currentTime = millis();
        if (currentTime - lastGSMCheck >= GSM_CHECK_INTERVAL) {
            lastGSMCheck = currentTime;
            checkIncomingGSMSMS();
        }
        
        // GPS transmission logic
        if (currentTime - lastTransmit >= TRANSMIT_INTERVAL) {
            lastTransmit = currentTime;
            
            // Get GPS data
            double lat, lon;
            String status;
            
            if (gpsState.hasValidFix) {
                lat = gpsState.latitude;
                lon = gpsState.longitude;
                status = "CURRENT";
            } else if (gpsState.hasLastLocation) {
                lat = gpsState.lastLatitude;
                lon = gpsState.lastLongitude;
                status = "LAST";
            } else {
                // Skip transmission if no GPS data available
                SerialBT.println("⏭️ No GPS data available, skipping transmission");
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }
            
            // Format GPS message
            String gpsMessage = "GPS " + status + ": ";
            gpsMessage += wtState.soldierID + ",";
            gpsMessage += String(lat, 6) + "," + String(lon, 6);
            
            // Attempt transmission with fallback: DMR -> LoRa -> GSM
            bool sent = false;
            
            // Try DMR first (fastest, most reliable for tactical)
            SerialBT.println("📡 Attempting DMR transmission...");
            if (dmr.sendSMS(0xFFFFFF, gpsMessage.c_str())) { // Broadcast to all
                SerialBT.println("✅ GPS sent via DMR");
                sent = true;
            }
            
            // Fallback to LoRa if DMR fails
            if (!sent && loraState.initialized) {
                SerialBT.println("📡 DMR failed, trying LoRa...");
                if (sendLoRaMessage(gpsMessage)) {
                    SerialBT.println("✅ GPS sent via LoRa");
                    sent = true;
                }
            }
            
            // Fallback to GSM if LoRa fails
            if (!sent && gsmState.initialized && gsmState.networkRegistered) {
                SerialBT.println("📡 LoRa failed, trying GSM...");
                // Send to configured phone number (add to settings)
                String phoneNumber = "+1234567890"; // TODO: Make configurable
                sendGSMFallbackSMS(phoneNumber, gpsMessage);
                SerialBT.println("✅ GPS sent via GSM");
                sent = true;
            }
            
            if (!sent) {
                SerialBT.println("❌ All transmission methods failed");
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second GPS update
    }
}

// Receiver mode task - Listen on all channels and output to Bluetooth
void receiverTask(void *parameter) {
    while (true) {
        // Check all communication channels for incoming messages
        
        // Check DMR for messages
        dmr.update();
        
        // Check LoRa messages
        if (loraState.initialized) {
            String loraMsg = loraReliable.receive();
            if (loraMsg.length() > 0) {
                SerialBT.println("\n📡 [LoRa] " + loraMsg);
                SerialBT.println("RSSI: " + String(loraReliable.getRSSI()) + " dBm");
            }
        }
        
        // Check GSM messages (less frequently to avoid spam)
        static unsigned long lastGSMCheck = 0;
        if (millis() - lastGSMCheck >= 5000) { // Check every 5 seconds
            lastGSMCheck = millis();
            checkIncomingGSMSMS();
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms polling
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
    
    SerialBT.println("🚀 Creating FreeRTOS tasks based on mode...");
    
    // Mode-specific task creation for optimization
    if (currentMode == MODE_TRACKER) {
        // TRACKER MODE: GPS + Communication + Bluetooth + Display + Keyboard
        SerialBT.println("📍 TRACKER MODE: GPS transmission with fallback");
        
        xTaskCreatePinnedToCore(trackerTask, "Tracker_Task", 4096, NULL, 5, &gpsTaskHandle, 1);
        xTaskCreatePinnedToCore(bluetoothTask, "BT_Task", BLUETOOTH_STACK_SIZE, NULL, 4, &bluetoothTaskHandle, 0);
        xTaskCreatePinnedToCore(displayTask, "Display_Task", DISPLAY_STACK_SIZE, NULL, DISPLAY_TASK_PRIORITY, &displayTaskHandle, 1);
        xTaskCreatePinnedToCore(keyboardTask, "Keyboard_Task", KEYBOARD_STACK_SIZE, NULL, KEYBOARD_TASK_PRIORITY, &keyboardTaskHandle, 1);
        
        SerialBT.println("✅ Tracker tasks created: GPS+Comms, Bluetooth, Display, Keyboard");
        SerialBT.println("⚡ Optimized: Disabled separate DMR/LoRa/GSM tasks (3 tasks saved)");
        
    } else if (currentMode == MODE_RECEIVER) {
        // RECEIVER MODE: Only Communication receiver + Bluetooth output
        SerialBT.println("📻 RECEIVER MODE: Multi-channel receiver");
        
        xTaskCreatePinnedToCore(receiverTask, "Receiver_Task", 4096, NULL, 5, &dmrTaskHandle, 0);
        xTaskCreatePinnedToCore(bluetoothTask, "BT_Task", BLUETOOTH_STACK_SIZE, NULL, 4, &bluetoothTaskHandle, 0);
        
        SerialBT.println("✅ Receiver tasks created: Multi-channel RX, Bluetooth");
        SerialBT.println("⚡ Optimized: Disabled GPS, Display, Keyboard");
        
    } else {
        // FULL MODE: All tasks for complete functionality
        SerialBT.println("🔧 FULL MODE: All subsystems active");
        
        xTaskCreatePinnedToCore(dmrTask, "DMR_Task", DMR_STACK_SIZE, NULL, DMR_TASK_PRIORITY, &dmrTaskHandle, 0);
        xTaskCreatePinnedToCore(gpsTask, "GPS_Task", GPS_STACK_SIZE, NULL, GPS_TASK_PRIORITY, &gpsTaskHandle, 1);
        xTaskCreatePinnedToCore(gsmTask, "GSM_Task", GSM_STACK_SIZE, NULL, GSM_TASK_PRIORITY, &gsmTaskHandle, 1);
        xTaskCreatePinnedToCore(loraTask, "LoRa_Task", LORA_STACK_SIZE, NULL, LORA_TASK_PRIORITY, &loraTaskHandle, 0);
        xTaskCreatePinnedToCore(bluetoothTask, "BT_Task", BLUETOOTH_STACK_SIZE, NULL, BLUETOOTH_TASK_PRIORITY, &bluetoothTaskHandle, 0);
        xTaskCreatePinnedToCore(displayTask, "Display_Task", DISPLAY_STACK_SIZE, NULL, DISPLAY_TASK_PRIORITY, &displayTaskHandle, 1);
        xTaskCreatePinnedToCore(keyboardTask, "Keyboard_Task", KEYBOARD_STACK_SIZE, NULL, KEYBOARD_TASK_PRIORITY, &keyboardTaskHandle, 1);
        
        SerialBT.println("✅ All 7 tasks created: DMR, GPS, GSM, LoRa, BT, Display, Keyboard");
    }
}

void loop() {
    // FreeRTOS scheduler handles everything
    // Keep loop alive to feed watchdog and maintain Bluetooth
    vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second delay
}