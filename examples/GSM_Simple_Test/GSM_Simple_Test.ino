/*
  Simple GSM Function Tester
  Direct implementation without external library dependencies
  
  Hardware Setup:
  - ESP32 GPIO25 -> GSM RX
  - ESP32 GPIO26 -> GSM TX
  - GSM GND -> ESP32 GND
  - GSM VCC -> Power supply (3.7V-4.2V)
  
  Commands via Bluetooth:
  - 'init' - Initialize GSM module
  - 'sms' - Send test SMS
  - 'check' - Check network registration
  - 'signal' - Get signal strength
  - 'status' - Show current status
  - 'call:+91xxxxxxxxxx' - Make a call
  - 'answer' - Answer incoming call
  - 'hangup' - Hang up call
*/

#define GSM_RX_PIN 26
#define GSM_TX_PIN 25

#include <BluetoothSerial.h>
#include <HardwareSerial.h>

BluetoothSerial SerialBT;
HardwareSerial gsmSerial(1);

// GSM State structure
struct GSMState {
    bool initialized = false;
    bool networkRegistered = false;
    int signalStrength = 0;
    String operatorName = "";
};

GSMState gsmState;
bool debugMode = true;

// Debug helper function
void debugPrint(String message) {
  if (debugMode) {
    SerialBT.println("[DEBUG] " + message);
    Serial.println("[DEBUG] " + message);
  }
}

// Wait for GSM module response
String waitForResponse(int timeout = 5000) {
  String response = "";
  unsigned long startTime = millis();
  
  while (millis() - startTime < timeout) {
    if (gsmSerial.available()) {
      response += gsmSerial.readString();
      delay(100);
    }
    if (response.indexOf("OK") >= 0 || response.indexOf("ERROR") >= 0) {
      break;
    }
  }
  
  debugPrint("GSM Response: " + response);
  return response;
}

// Send AT command
bool sendATCommand(String command, String expectedResponse = "OK", int timeout = 5000) {
  debugPrint("Sending: " + command);
  
  gsmSerial.println(command);
  String response = waitForResponse(timeout);
  
  bool success = response.indexOf(expectedResponse) >= 0;
  debugPrint("Command " + command + " " + (success ? "SUCCESS" : "FAILED"));
  
  return success;
}

void initGSMModule() {
  SerialBT.println("\\n🔄 Initializing GSM Module...");
  debugPrint("Starting GSM initialization");
  
  // Basic AT commands to initialize
  if (sendATCommand("AT")) {
    SerialBT.println("✅ GSM module responding");
    
    // Set text mode for SMS
    sendATCommand("AT+CMGF=1");
    
    // Check network registration
    checkGSMNetwork();
    
    // Get signal strength
    getGSMSignalStrength();
    
    // Get service provider
    getServiceProvider();
    
    gsmState.initialized = true;
    SerialBT.println("✅ GSM module initialized successfully");
  } else {
    SerialBT.println("❌ GSM module not responding");
    gsmState.initialized = false;
  }
}

void checkGSMNetwork() {
  SerialBT.println("\\n🔍 Checking network registration...");
  
  gsmSerial.println("AT+CREG?");
  String response = waitForResponse();
  
  if (response.indexOf("+CREG: 0,1") >= 0 || response.indexOf("+CREG: 0,5") >= 0) {
    gsmState.networkRegistered = true;
    SerialBT.println("✅ Network registered");
  } else {
    gsmState.networkRegistered = false;
    SerialBT.println("❌ Network not registered");
    debugPrint("Network response: " + response);
  }
}

void getGSMSignalStrength() {
  SerialBT.println("\\n📶 Getting signal strength...");
  
  gsmSerial.println("AT+CSQ");
  String response = waitForResponse();
  
  int csqIndex = response.indexOf("+CSQ: ");
  if (csqIndex >= 0) {
    String csqValue = response.substring(csqIndex + 6);
    int commaIndex = csqValue.indexOf(",");
    if (commaIndex >= 0) {
      int signal = csqValue.substring(0, commaIndex).toInt();
      gsmState.signalStrength = signal;
      
      if (signal == 99) {
        SerialBT.println("Signal: Not known or not detectable");
      } else {
        SerialBT.println("Signal Strength: " + String(signal) + " (0-31 scale)");
        int dbm = -113 + (signal * 2);
        SerialBT.println("Signal: " + String(dbm) + " dBm");
      }
    }
  } else {
    SerialBT.println("❌ Could not get signal strength");
  }
}

void getServiceProvider() {
  SerialBT.println("\\n📱 Getting service provider...");
  
  gsmSerial.println("AT+COPS?");
  String response = waitForResponse();
  
  int copsIndex = response.indexOf("+COPS: ");
  if (copsIndex >= 0) {
    String copsData = response.substring(copsIndex + 7);
    int firstQuote = copsData.indexOf('"');
    int secondQuote = copsData.indexOf('"', firstQuote + 1);
    
    if (firstQuote >= 0 && secondQuote >= 0) {
      gsmState.operatorName = copsData.substring(firstQuote + 1, secondQuote);
      SerialBT.println("Provider: " + gsmState.operatorName);
    } else {
      SerialBT.println("❌ Could not parse provider name");
    }
  } else {
    SerialBT.println("❌ No service provider found");
  }
}

void showStatus() {
  SerialBT.println("\\n📊 GSM Status:");
  SerialBT.println("  Initialized: " + String(gsmState.initialized ? "Yes" : "No"));
  SerialBT.println("  Network Registered: " + String(gsmState.networkRegistered ? "Yes" : "No"));
  SerialBT.println("  Signal Strength: " + String(gsmState.signalStrength));
  SerialBT.println("  Service Provider: " + gsmState.operatorName);
  SerialBT.println("  Debug Mode: " + String(debugMode ? "ON" : "OFF"));
  SerialBT.println("  Uptime: " + String(millis()/1000) + " seconds");
}

void sendTestSMS() {
  SerialBT.println("\\n📱 Sending test SMS...");
  
  if (!gsmState.initialized) {
    SerialBT.println("❌ GSM not initialized");
    return;
  }
  
  String phoneNumber = "+918667399071";
  String message = "Hello from ESP32! Signal: " + String(gsmState.signalStrength);
  
  // Set SMS to text mode
  if (sendATCommand("AT+CMGF=1")) {
    // Send SMS command
    gsmSerial.println("AT+CMGS=\\"" + phoneNumber + "\\"");
    delay(1000);
    
    // Send message
    gsmSerial.print(message);
    delay(500);
    gsmSerial.write(26); // Ctrl+Z to send SMS
    
    String response = waitForResponse(15000);
    if (response.indexOf("+CMGS:") >= 0) {
      SerialBT.println("✅ SMS sent successfully!");
    } else {
      SerialBT.println("❌ SMS sending failed");
      debugPrint("SMS Response: " + response);
    }
  } else {
    SerialBT.println("❌ Failed to set SMS text mode");
  }
}

void makeCall(String number) {
  SerialBT.println("\\n📞 Making call to: " + number);
  
  if (!gsmState.initialized) {
    SerialBT.println("❌ GSM not initialized");
    return;
  }
  
  String dialCommand = "ATD" + number + ";";
  if (sendATCommand(dialCommand, "OK", 10000)) {
    SerialBT.println("✅ Call initiated");
  } else {
    SerialBT.println("❌ Call failed");
  }
}

void answerCall() {
  SerialBT.println("\\n📞 Answering call...");
  
  if (sendATCommand("ATA")) {
    SerialBT.println("✅ Call answered");
  } else {
    SerialBT.println("❌ Failed to answer call");
  }
}

void hangupCall() {
  SerialBT.println("\\n📞 Hanging up call...");
  
  if (sendATCommand("ATH")) {
    SerialBT.println("✅ Call ended");
  } else {
    SerialBT.println("❌ Failed to hang up");
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("GSM_Simple_Tester");
  gsmSerial.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  
  delay(2000);
  SerialBT.println("=== Simple GSM Tester ===");
  SerialBT.println("🔧 Hardware Info:");
  SerialBT.println("   GSM RX: GPIO" + String(GSM_RX_PIN));
  SerialBT.println("   GSM TX: GPIO" + String(GSM_TX_PIN));
  SerialBT.println("   Baud Rate: 9600");
  SerialBT.println();
  SerialBT.println("📋 Commands:");
  SerialBT.println("  'init' - Initialize GSM module");
  SerialBT.println("  'sms' - Send test SMS");
  SerialBT.println("  'check' - Check network");
  SerialBT.println("  'signal' - Get signal strength"); 
  SerialBT.println("  'provider' - Get service provider");
  SerialBT.println("  'status' - Show current status");
  SerialBT.println("  'call:+91xxxxxxxx' - Make a call");
  SerialBT.println("  'answer' - Answer incoming call");
  SerialBT.println("  'hangup' - Hang up call");
  SerialBT.println("  'debug' - Toggle debug mode");
  SerialBT.println("================================");
  
  // Initial GSM module initialization
  initGSMModule();
}

void loop() {
  // Handle Bluetooth commands
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\\n');
    command.trim();
    
    debugPrint("Received command: '" + command + "'");
    
    if (command == "init") {
      initGSMModule();
    } else if (command == "sms") {
      sendTestSMS();
    } else if (command == "check") {
      checkGSMNetwork();
    } else if (command == "signal") {
      getGSMSignalStrength();
    } else if (command == "provider") {
      getServiceProvider();
    } else if (command == "status") {
      showStatus();
    } else if (command.startsWith("call:")) {
      String number = command.substring(5);
      makeCall(number);
    } else if (command == "answer") {
      answerCall();
    } else if (command == "hangup") {
      hangupCall();
    } else if (command == "debug") {
      debugMode = !debugMode;
      SerialBT.println("🔧 Debug mode: " + String(debugMode ? "ON" : "OFF"));
    } else if (command.length() > 0) {
      SerialBT.println("❌ Unknown command: " + command);
      SerialBT.println("Type a valid command (see list above)");
    }
  }
  
  // Check for incoming calls
  if (gsmSerial.available()) {
    String gsmResponse = gsmSerial.readString();
    if (gsmResponse.indexOf("RING") >= 0) {
      SerialBT.println("📞 Incoming call detected!");
      SerialBT.println("Type 'answer' to answer or 'hangup' to reject");
    }
    debugPrint("GSM Activity: " + gsmResponse);
  }
  
  // Small delay
  delay(100);
}