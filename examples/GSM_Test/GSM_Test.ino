/*
  GSM Auto SMS Sender - SIM800L Library
  Sends "helloo" SMS every 5 seconds using SIM800L library
  
  Hardware Setup:
  - ESP32 GPIO25 -> GSM RX
  - ESP32 GPIO26 -> GSM TX
  - GSM GND -> ESP32 GND
  - GSM VCC -> Power supply (3.7V-4.2V)
*/

#define GSM_RX_PIN 26
#define GSM_TX_PIN 25

#include <BluetoothSerial.h>
#include <HardwareSerial.h>
#include "SIM800L.h"

BluetoothSerial SerialBT;
HardwareSerial gsmSerial(1);
SIM800L gsm;

// GSM State structure
struct GSMState {
    bool initialized = false;
    bool networkRegistered = false;
    int signalStrength = 0;
    String operatorName = "";
};

GSMState gsmState;
bool debugMode = true;
unsigned long lastSMSTime = 0;
const unsigned long SMS_INTERVAL = 10000; // 10 seconds

// Debug helper function
void debugPrint(String message) {
  if (debugMode) {
    SerialBT.println("[DEBUG] " + message);
    Serial.println("[DEBUG] " + message);
  }
}

void initGSMModule() {
  SerialBT.println("\n🔄 Initializing GSM Module...");
  debugPrint("Starting SIM800L initialization");
  
  if (gsm.begin(gsmSerial)) {
    SerialBT.println("✅ GSM module initialized successfully");
    gsmState.initialized = true;
    
    // Check network registration
    if (gsm.checkNetwork()) {
      gsmState.networkRegistered = true;
      SerialBT.println("✅ Network registered");
    } else {
      SerialBT.println("❌ Network not registered");
    }
    
    SerialBT.println("📱 Ready to send SMS every 10 seconds!");
  } else {
    SerialBT.println("❌ GSM module initialization failed");
    gsmState.initialized = false;
  }
}

void sendAutoSMS() {
  if (!gsmState.initialized || !gsmState.networkRegistered) {
    SerialBT.println("❌ GSM not ready for SMS");
    return;
  }
  
  char phoneNumber[] = "8667399071";
  char message[] = "helloo";
  
  debugPrint("Sending auto SMS: " + String(message));
  SerialBT.println("📱 Sending SMS: " + String(message));
  
  if (gsm.sendSMS(phoneNumber, message)) {
    SerialBT.println("✅ SMS sent successfully!");
  } else {
    SerialBT.println("❌ SMS sending failed");
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("GSM_Auto_SMS");
  gsmSerial.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  
  delay(2000);
  SerialBT.println("=== GSM Auto SMS Sender ===");
  SerialBT.println("🔧 Hardware Info:");
  SerialBT.println("   GSM RX: GPIO" + String(GSM_RX_PIN));
  SerialBT.println("   GSM TX: GPIO" + String(GSM_TX_PIN));
  SerialBT.println("   Baud Rate: 9600");
  SerialBT.println("   Message: helloo");
  SerialBT.println("   Interval: 10 seconds");
  SerialBT.println("================================");
  
  // Initial GSM module initialization
  initGSMModule();
}

void loop() {
  // Send SMS every 5 seconds
  if (millis() - lastSMSTime >= SMS_INTERVAL) {
    sendAutoSMS();
    lastSMSTime = millis();
  }
  
  // Handle Bluetooth commands for debugging
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    command.trim();
    
    debugPrint("Received command: '" + command + "'");
    
    if (command == "init") {
      initGSMModule();
    } else if (command == "debug") {
      debugMode = !debugMode;
      SerialBT.println("🔧 Debug mode: " + String(debugMode ? "ON" : "OFF"));
    } else if (command == "status") {
      SerialBT.println("📊 Status:");
      SerialBT.println("  Initialized: " + String(gsmState.initialized ? "Yes" : "No"));
      SerialBT.println("  Network: " + String(gsmState.networkRegistered ? "Yes" : "No"));
      SerialBT.println("  Uptime: " + String(millis()/1000) + " seconds");
    } else if (command.length() > 0) {
      SerialBT.println("❌ Available commands: init, debug, status");
    }
  }
  
  // Small delay
  delay(100);
}

