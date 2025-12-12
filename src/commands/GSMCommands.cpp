#include "GSMCommands.h"
#include "../../include/WalkieTalkie.h"
#include "../managers/GSMManager.h"

extern GSMState gsmState;
extern WalkieTalkieState wtState;

String sendGSMATCommand(const String& cmd, unsigned long timeout) {
    Serial1.println(cmd);
    unsigned long start = millis();
    String response = "";
    while (millis() - start < timeout) {
        while (Serial1.available()) {
            char c = Serial1.read();
            response += c;
        }
        delay(10);
    }
    response.trim();
    return response;
}

void refreshGSMState() {
    String atResp = sendGSMATCommand("AT", 500);
    gsmState.initialized = atResp.indexOf("OK") != -1;
    String cregResp = sendGSMATCommand("AT+CREG?", 800);
    int regIdx = cregResp.indexOf(",");
    gsmState.networkRegistered = false;
    if (regIdx != -1 && regIdx + 1 < cregResp.length()) {
        char regStatus = cregResp.charAt(regIdx + 1);
        gsmState.networkRegistered = (regStatus == '1' || regStatus == '5');
    }
    String csqResp = sendGSMATCommand("AT+CSQ", 800);
    int csqIdx = csqResp.indexOf(":");
    gsmState.signalStrength = 0;
    if (csqIdx != -1) {
        int commaIdx = csqResp.indexOf(",", csqIdx);
        if (commaIdx != -1) {
            String rssiStr = csqResp.substring(csqIdx + 1, commaIdx);
            rssiStr.trim();
            int rssi = rssiStr.toInt();
            if (rssi >= 0 && rssi <= 31) {
                gsmState.signalStrength = rssi;
            }
        }
    }
}

void handleGSMCommand(Stream* stream, String command) {
    if (command == "gsmstatus") {
        refreshGSMState();
        stream->println("\n📱 GSM Status:");
        stream->print("Initialized: ");
        stream->println(gsmState.initialized ? "YES" : "NO");
        stream->print("Network: ");
        stream->println(gsmState.networkRegistered ? "REGISTERED" : "NOT REGISTERED");
        stream->print("Signal: ");
        stream->print(gsmState.signalStrength);
        stream->println("/31");
        stream->print("Phone: ");
        stream->println(gsmState.phoneNumber.length() > 0 ? gsmState.phoneNumber : "Not set");
        if (gsmState.operatorName.length() > 0) {
            stream->print("Operator: ");
            stream->println(gsmState.operatorName);
        }
    }
    else if (command.startsWith("gsmcmd ")) {
        String atCmd = command.substring(7);
        atCmd.trim();
        if (atCmd.length() == 0) {
            stream->println("❌ Usage: gsmcmd <AT_command>");
            stream->println("Example: gsmcmd AT");
            stream->println("Example: gsmcmd AT+CSQ");
            return;
        }
        stream->print("📱 Sending to GSM: ");
        stream->println(atCmd);
        stream->println("---");
        Serial1.println(atCmd);
        delay(500);
        unsigned long timeout = 2000;
        unsigned long startTime = millis();
        String response = "";
        while (millis() - startTime < timeout && Serial1.available()) {
            char c = Serial1.read();
            response += c;
            stream->write(c);
            delay(2);
        }
        if (response.length() == 0) {
            stream->println("\n❌ No response from GSM module");
        } else {
            stream->println("\n---");
        }
    }
    else if (command.startsWith("gsmphone ")) {
        String phone = command.substring(9);
        phone.trim();
        if (phone.length() > 0) {
            gsmState.phoneNumber = phone;
            stream->println("✅ Fallback phone number set: " + phone);
        } else {
            stream->println("❌ Invalid phone number");
        }
    }
    else if (command.startsWith("gsmsms ")) {
        int spaceIndex = command.indexOf(' ', 7);
        if (spaceIndex != -1) {
            String phone = command.substring(7, spaceIndex);
            String message = command.substring(spaceIndex + 1);
            phone.trim();
            message.trim();
            if (phone.length() > 0 && message.length() > 0) {
                wtState.currentSendMode = "GSM";
                extern void sendGSMFallbackSMS(String phoneNumber, String message);
                sendGSMFallbackSMS(phone, message);
            } else {
                stream->println("❌ Invalid phone number or message");
            }
        } else {
            stream->println("❌ Format: gsmsms <number> <message>");
        }
    }
}
