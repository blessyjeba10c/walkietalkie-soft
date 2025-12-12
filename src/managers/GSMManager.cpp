#include "GSMManager.h"
#include "BluetoothSerial.h"
#include "../SIM800L.h"
#include "WalkieTalkie.h"

extern BluetoothSerial SerialBT;
extern HardwareSerial Serial1;

GSMState gsmState;
SIM800L gsm;

void initializeGSM() {
    SerialBT.println("📱 Initializing GSM module with SIM800L library...");
    
    if (gsm.begin(Serial1)) {
        SerialBT.println("✅ GSM module initialized successfully");
        gsmState.initialized = true;
        
        // Check network registration
        if (gsm.checkNetwork()) {
            gsmState.networkRegistered = true;
            SerialBT.println("✅ Network registered");
            
            // Get signal strength
            int8_t signal = gsm.signalStrength();
            if (signal >= 0) {
                gsmState.signalStrength = signal;
                SerialBT.println("Signal Strength: " + String(signal));
            }
            
            // Get service provider
            String provider = gsm.serviceProvider();
            if (provider.length() > 0) {
                SerialBT.println("Provider: " + provider);
            }
        } else {
            gsmState.networkRegistered = false;
            SerialBT.println("❌ Network not registered");
        }
        
        SerialBT.println("✅ GSM module ready");
    } else {
        SerialBT.println("❌ GSM module initialization failed");
        gsmState.initialized = false;
        gsmState.networkRegistered = false;
    }
}

bool waitForGSMResponse(String expectedResponse, unsigned long timeout) {
    unsigned long startTime = millis();
    String response = "";
    
    while (millis() - startTime < timeout) {
        if (Serial1.available()) {
            response += (char)Serial1.read();
            if (response.indexOf(expectedResponse) != -1) {
                return true;
            }
        }
        delay(10);
    }
    return false;
}

void checkGSMNetwork() {
    SerialBT.println("🔍 Checking network registration...");
    
    if (gsm.checkNetwork()) {
        gsmState.networkRegistered = true;
        SerialBT.println("✅ GSM network registered");
        
        // Get signal strength
        getGSMSignalStrength();
        
        // Get service provider
        String provider = gsm.serviceProvider();
        if (provider.length() > 0 && provider != "No network") {
            SerialBT.println("Provider: " + provider);
        }
    } else {
        gsmState.networkRegistered = false;
        SerialBT.println("❌ GSM network not registered");
    }
}

void getGSMSignalStrength() {
    SerialBT.println("📶 Getting signal strength...");
    
    int8_t signal = gsm.signalStrength();
    if (signal >= 0) {
        gsmState.signalStrength = signal;
        SerialBT.println("Signal Strength: " + String(signal) + " (0-31 scale)");
        
        // Convert to dBm for better understanding
        if (signal == 99) {
            SerialBT.println("Signal: Not known or not detectable");
        } else {
            int dbm = -113 + (signal * 2);
            SerialBT.println("Signal: " + String(dbm) + " dBm");
        }
    } else {
        SerialBT.println("❌ Could not get signal strength");
    }
}

void sendGSMFallbackSMS(String phoneNumber, String message) {
    // First check current status
    if (!gsmState.initialized || !gsmState.networkRegistered) {
        SerialBT.println("🔄 GSM status check - reinitializing...");
        
        // Try to reinitialize GSM
        initializeGSM();
        
        // If still not ready after reinit, give up
        if (!gsmState.initialized || !gsmState.networkRegistered) {
            SerialBT.println("❌ GSM not ready for SMS after recheck");
            SerialBT.print("   Initialized: ");
            SerialBT.println(gsmState.initialized ? "Yes" : "No");
            SerialBT.print("   Network Registered: ");
            SerialBT.println(gsmState.networkRegistered ? "Yes" : "No");
            return;
        } else {
            SerialBT.println("✅ GSM ready after reinitialization");
        }
    }
    
    SerialBT.println("📱 Sending SMS via GSM (SIM800L library)...");
    SerialBT.println("To: " + phoneNumber);
    SerialBT.println("Message: " + message);
    
    // Convert String to char array for SIM800L library
    char phoneNumberArray[20];
    char messageArray[160];
    
    phoneNumber.toCharArray(phoneNumberArray, 20);
    message.toCharArray(messageArray, 160);
    
    if (gsm.sendSMS(phoneNumberArray, messageArray)) {
        SerialBT.println("✅ SMS sent successfully via SIM800L library");
    } else {
        SerialBT.println("❌ Failed to send SMS via SIM800L library");
    }
}

void checkIncomingGSMSMS() {
    // Queue-based SMS checking (more reliable than event notifications)
    // Query for all unread messages
    Serial1.println("AT+CMGL=\"REC UNREAD\"");
    delay(500); // Wait for response
    
    String response = "";
    unsigned long startTime = millis();
    while (millis() - startTime < 1000 && Serial1.available()) {
        response += Serial1.readString();
        delay(10);
    }
    
    // Check if any messages were found
    if (response.indexOf("+CMGL:") != -1) {
        int index = 0;
        int messagesFound = 0;
        
        while (true) {
            int msgStart = response.indexOf("+CMGL:", index);
            if (msgStart == -1) break;
            
            // Extract message index for deletion
            int indexStart = msgStart + 7;
            int indexEnd = response.indexOf(',', indexStart);
            String msgIndexStr = response.substring(indexStart, indexEnd);
            msgIndexStr.trim();
            
            // Extract sender number
            int phoneStart = response.indexOf('"', indexEnd) + 1;
            int phoneEnd = response.indexOf('"', phoneStart);
            String senderNumber = response.substring(phoneStart, phoneEnd);
            
            // Extract message body (next line after +CMGL)
            int bodyStart = response.indexOf('\n', msgStart) + 1;
            int bodyEnd = response.indexOf('\n', bodyStart);
            if (bodyEnd == -1) bodyEnd = response.length();
            String messageBody = response.substring(bodyStart, bodyEnd);
            messageBody.trim();
            
            // Process valid messages
            if (messageBody.length() > 0 && 
                !messageBody.startsWith("OK") && 
                !messageBody.startsWith("+CMGL") &&
                !messageBody.startsWith("AT+")) {
                
                messagesFound++;
                
                SerialBT.println("\n📱 GSM SMS RECEIVED");
                SerialBT.println("From: " + senderNumber);
                SerialBT.println("Message: " + messageBody);
                SerialBT.println("Index: " + msgIndexStr);
                SerialBT.println("━━━━━━━━━━━━━━━━━━━━━━\n");
                
                // Add to message history
                extern void addMessage(String message);
                String historyEntry = "GSM:" + messageBody;
                addMessage(historyEntry);
                
                // Check if this is a GPS message and parse it
                if (messageBody.startsWith("GPS ")) {
                    extern void parseIncomingGPS(String message, String commMode);
                    parseIncomingGPS(messageBody, "GSM");
                } else {
                    // Non-GPS message - add to display overlay queue in tracker mode
                    extern DemoMode currentMode;
                    if (currentMode == MODE_TRACKER) {
                        extern void addMessageToQueue(String message);
                        String displayMsg = "[GSM] " + messageBody;
                        addMessageToQueue(displayMsg);
                    }
                }
                
                // Delete message after reading
                delay(100);
                Serial1.println("AT+CMGD=" + msgIndexStr);
                delay(200);
                
                SerialBT.println("[GSM] Deleted message " + msgIndexStr);
            }
            
            index = bodyEnd + 1;
            if (index >= response.length()) break;
        }
        
        if (messagesFound > 0) {
            SerialBT.println("[GSM] Processed " + String(messagesFound) + " message(s)");
        }
    }
}

void readGSMSMS(int index) {
    SerialBT.println("📱 Reading SMS at index: " + String(index));
    
    // Use SIM800L library to read SMS
    String smsContent = gsm.readSMS(index);
    
    if (smsContent.length() > 0) {
        SerialBT.println("📱 GSM SMS Received:");
        SerialBT.println("Raw content: " + smsContent);
        
        // Parse SMS content from SIM800L library format
        // Format: +CMGR: "REC UNREAD","+phoneNumber","","timestamp"\r\nMessage content
        if (smsContent.indexOf("+CMGR:") != -1) {
            int messageStart = smsContent.indexOf('\n', smsContent.indexOf("+CMGR:"));
            if (messageStart != -1) {
                String message = smsContent.substring(messageStart + 1);
                message.trim();
                
                SerialBT.println("Parsed message: " + message);
                
                // Add to message history
                extern void addMessage(String message);
                String historyEntry = "GSM:" + message;
                addMessage(historyEntry);
                
                // Check if this is a GPS message and parse it
                if (message.startsWith("GPS ")) {
                    // Need to include WalkieTalkie.h functions
                    extern void parseIncomingGPS(String message, String commMode);
                    parseIncomingGPS(message, "GSM");
                }
            }
        }
    } else {
        SerialBT.println("❌ Failed to read SMS at index " + String(index));
    }
}