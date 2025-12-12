#include "LoRaCommands.h"
#include "../../include/WalkieTalkie.h"
#include "../managers/LoRaManager.h"
#include "../managers/GPSManager.h"

extern WalkieTalkieState wtState;
extern LoRaState loraState;
extern GPSState gpsState;

void handleLoRaCommand(Stream* stream, String command) {
    if (command == "lorastatus") {
        stream->println("\\n📡 LoRa Status:");
        stream->print("Initialized: ");
        stream->println(loraState.initialized ? "YES" : "NO");
        stream->print("Available: ");
        stream->println(loraState.available ? "YES" : "NO");
        stream->print("ACK Mode: ");
        stream->println(loraState.ackMode ? "ENABLED" : "DISABLED");
        if (loraState.ackMode) {
            stream->print("Max Retries: ");
            stream->println(loraState.maxRetries);
            stream->print("ACK Timeout: ");
            stream->print(loraState.ackTimeout);
            stream->println(" ms");
        }
        if (loraState.initialized) {
            stream->print("Last RSSI: ");
            stream->print(loraState.rssi);
            stream->println(" dBm");
            stream->print("Last SNR: ");
            stream->print(loraState.snr);
            stream->println(" dB");
            stream->print("Frequency: 433 MHz");
            stream->println();
            if (loraState.lastMessage.length() > 0) {
                stream->print("Last Message: ");
                stream->println(loraState.lastMessage);
            }
        }
    }
    else if (command == "ackon") {
        enableLoRaAck();
        stream->println("✅ LoRa ACK mode enabled");
        stream->println("Messages will wait for acknowledgment");
    }
    else if (command == "ackoff") {
        disableLoRaAck();
        stream->println("✅ LoRa ACK mode disabled");
        stream->println("Messages will be broadcast without ACK");
    }
    else if (command.startsWith("lorasms ")) {
        String message = command.substring(8);
        message.trim();
        
        // Check for --no-id flag
        bool includeID = true;
        if (message.endsWith(" --no-id")) {
            includeID = false;
            message = message.substring(0, message.length() - 8);
            message.trim();
        }
        
        if (message.length() > 0) {
            // Add device ID if requested
            String finalMessage = message;
            if (includeID) {
                finalMessage = "[ID:" + String(wtState.soldierID) + "] " + message;
            }
            
            if (sendLoRaMessage(finalMessage)) {
                stream->println("✅ LoRa message sent: " + finalMessage);
            } else {
                stream->println("❌ Failed to send LoRa message");
            }
        } else {
            stream->println("❌ Format: lorasms <message> [--no-id]");
            stream->println("   Use --no-id flag to send without device ID");
        }
    }
    else if (command.startsWith("loragps ")) {
        String targetStr = command.substring(8);
        targetStr.trim();
        if (targetStr.length() > 0) {
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
                status = "LAST GPS";
            } else {
                lat = 29.938971327453903;
                lon = 77.56449807342506;
                status = "DEFAULT";
            }
            
            // Create GPS message with soldier ID
            String gpsMessage = "GPS " + status + ": ";
            gpsMessage += wtState.soldierID + ",";
            gpsMessage += String(lat, 6) + "," + String(lon, 6);
            gpsMessage += " [TO:" + targetStr + "]";
            
            if (sendLoRaMessage(gpsMessage)) {
                stream->print("📍 GPS sent via LoRa to ");
                stream->print(targetStr);
                stream->print(" (");
                stream->print(status);
                stream->print("): ");
                stream->print(lat, 6);
                stream->print(", ");
                stream->println(lon, 6);
            } else {
                stream->println("❌ Failed to send GPS via LoRa");
            }
        } else {
            stream->println("❌ Format: loragps <target_callsign>");
        }
    }
}