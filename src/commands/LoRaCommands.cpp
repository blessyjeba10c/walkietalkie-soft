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
    else if (command.startsWith("lorasms ")) {
        String message = command.substring(8);
        message.trim();
        if (message.length() > 0) {
            if (sendLoRaMessage(message)) {
                stream->println("✅ LoRa message sent: " + message);
            } else {
                stream->println("❌ Failed to send LoRa message");
            }
        } else {
            stream->println("❌ Format: lorasms <message>");
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