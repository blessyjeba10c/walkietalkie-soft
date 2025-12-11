/*
  GSM Function Tester
  Test GSM functions with Bluetooth control
  
  Hardware Setup:
  - ESP32 GPIO25 -> GSM RX
  - ESP32 GPIO26 -> GSM TX
  - GSM GND -> ESP32 GND
  - GSM VCC -> Power supply
  
  Commands via Bluetooth:
  - 'sms' - Send test SMS
  - 'check' - Check network registration
  - 'signal' - Get signal strength
  - 'wait:OK' - Test waitForGSMResponse
  - Or type AT commands directly
*/

#define GSM_RX_PIN 26
#define GSM_TX_PIN 25

#include <BluetoothSerial.h>
#include <HardwareSerial.h>

BluetoothSerial SerialBT;
HardwareSerial gsmSerial(1);

// GSM State structure (from your main code)
struct GSMState {
    bool initialized = false;
    bool networkRegistered = false;
    int signalStrength = 0;
    String operatorName = "";
};

GSMState gsmState;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("GSM_Tester");
  gsmSerial.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  
  delay(1000);
  SerialBT.println("=== GSM Function Tester ===");
  SerialBT.println("Commands:");
  SerialBT.println("  'sms' - Send test SMS");
  SerialBT.println("  'check' - Check network");
  SerialBT.println("  'signal' - Get signal strength"); 
  SerialBT.println("  'wait:OK' - Test wait function");
  SerialBT.println("  Or type AT commands directly");
  SerialBT.println("========================");
}

void loop() {
  // Handle Bluetooth commands
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    command.trim();
    
    if (command == "sms") {
      SendMessage();
    } else if (command == "check") {
      checkGSMNetwork();
    } else if (command == "signal") {
      getGSMSignalStrength();
      SerialBT.println("Signal Strength: " + String(gsmState.signalStrength));
    } else if (command.startsWith("wait:")) {
      String response = command.substring(5);
      SerialBT.println("Testing waitForGSMResponse for: " + response);
      gsmSerial.println("AT");
      bool result = waitForGSMResponse(response, 3000);
      SerialBT.println("Result: " + String(result ? "SUCCESS" : "FAILED"));
    } else if (command.length() > 0) {
      // Send AT command directly
      SerialBT.println(">>> " + command);
      gsmSerial.println(command);
    }
  }
  
  // Forward GSM responses to Bluetooth
  if (gsmSerial.available()) {
    String response = "";
    while (gsmSerial.available()) {
      char c = gsmSerial.read();
      response += c;
      delay(1);
    }
    if (response.length() > 0) {
      SerialBT.print("<<< ");
      SerialBT.print(response);
    }
  }
}

// Your functions to test
bool waitForGSMResponse(String expectedResponse, unsigned long timeout) {
    SerialBT.println("Waiting for: '" + expectedResponse + "' (timeout: " + String(timeout) + "ms)");
    unsigned long startTime = millis();
    String response = "";
    
    while (millis() - startTime < timeout) {
        if (gsmSerial.available()) {
            char c = gsmSerial.read();
            response += c;
            SerialBT.print(c); // Show response in real-time
            if (response.indexOf(expectedResponse) != -1) {
                SerialBT.println("\n✅ Found expected response!");
                return true;
            }
        }
        delay(10);
    }
    SerialBT.println("\n❌ Timeout waiting for response");
    SerialBT.println("Full response: '" + response + "'");
    return false;
}

void checkGSMNetwork() {
    SerialBT.println("🔍 Checking GSM network registration...");
    
    // Check network registration
    gsmSerial.println("AT+CREG?");
    delay(500);
    
    String response = "";
    unsigned long startTime = millis();
    while (millis() - startTime < 2000) {
        if (gsmSerial.available()) {
            response += (char)gsmSerial.read();
            delay(10);
        }
    }
    
    SerialBT.println("CREG Response: '" + response + "'");
    
    if (response.indexOf("+CREG: 0,1") != -1 || response.indexOf("+CREG: 0,5") != -1) {
        gsmState.networkRegistered = true;
        SerialBT.println("✅ GSM network registered");
        
        // Get operator name
        SerialBT.println("Getting operator info...");
        gsmSerial.println("AT+COPS?");
        delay(500);
        
        // Read operator response
        String opResponse = "";
        startTime = millis();
        while (millis() - startTime < 2000) {
            if (gsmSerial.available()) {
                opResponse += (char)gsmSerial.read();
                delay(10);
            }
        }
        SerialBT.println("COPS Response: '" + opResponse + "'");
        
        // Get signal strength
        getGSMSignalStrength();
    } else {
        gsmState.networkRegistered = false;
        SerialBT.println("❌ GSM network not registered");
        
        // Show possible values for debugging
        SerialBT.println("Expected: '+CREG: 0,1' or '+CREG: 0,5'");
        if (response.indexOf("+CREG:") != -1) {
            SerialBT.println("Found CREG response but status not registered");
        } else {
            SerialBT.println("No CREG response found");
        }
    }
}

void getGSMSignalStrength() {
    SerialBT.println("📶 Getting signal strength...");
    
    gsmSerial.println("AT+CSQ");
    delay(500);
    
    String response = "";
    unsigned long startTime = millis();
    while (millis() - startTime < 1000) {
        if (gsmSerial.available()) {
            response += (char)gsmSerial.read();
            delay(10);
        }
    }
    
    SerialBT.println("CSQ Response: '" + response + "'");
    
    int csqIndex = response.indexOf("+CSQ: ");
    if (csqIndex != -1) {
        int commaIndex = response.indexOf(",", csqIndex);
        if (commaIndex != -1) {
            String rssiStr = response.substring(csqIndex + 6, commaIndex);
            int rssi = rssiStr.toInt();
            SerialBT.println("Raw RSSI: " + String(rssi));
            if (rssi != 99) {
                gsmState.signalStrength = rssi;
                SerialBT.println("✅ Signal strength: " + String(rssi));
            } else {
                SerialBT.println("❌ No signal (RSSI = 99)");
            }
        }
    } else {
        SerialBT.println("❌ No CSQ response found");
    }
}

void SendMessage() {
   SerialBT.println("📱 Sending test SMS...");
   SerialBT.println("Setting the GSM in text mode");
   gsmSerial.println("AT+CMGF=1\r");
   delay(2000);
   
   SerialBT.println("Sending SMS to the desired phone number!");
   gsmSerial.println("AT+CMGS=\"+918667399071\"\r");
   delay(2000);

   gsmSerial.println("Hello from ESP32 GSM Test");    // SMS Text
   delay(200);
   gsmSerial.println((char)26);               // ASCII code of CTRL+Z
   delay(2000);
   SerialBT.println("✅ SMS command sequence completed");
}