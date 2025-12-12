#include "GPSCommands.h"
#include "../../include/WalkieTalkie.h"
#include "../managers/GPSManager.h"
#include "DMR828S.h"

extern WalkieTalkieState wtState;
extern DMR828S dmr;
extern GPSState gpsState;

void handleGPSCommand(Stream* stream, String command) {
    if (command.startsWith("gps ")) {
        // GPS command: "gps <hex_id>"
        String idStr = command.substring(4);
        uint32_t targetID = strtoul(idStr.c_str(), NULL, 16);
        
        if (targetID > 0) {
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
                lat = 29.863594615284676;
                lon = 77.89650459615501;
                status = "DEFAULT";
            }
            
            // Create GPS message with soldier ID
            String gpsMessage = "GPS " + status + ": ";
            gpsMessage += wtState.soldierID + ",";
            gpsMessage += String(lat, 6) + "," + String(lon, 6);
            
            // Send via SMS
            if (dmr.sendSMS(targetID, gpsMessage.c_str())) {
                stream->print("📍 GPS sent to 0x");
                stream->print(targetID, HEX);
                stream->print(" (");
                stream->print(status);
                stream->print("): ");
                stream->print(lat, 6);
                stream->print(", ");
                stream->println(lon, 6);
            } else {
                stream->println("❌ GPS SMS send failed");
            }
        } else {
            stream->println("❌ Invalid target ID. Use: gps <hex_id>");
        }
    }
    else if (command.startsWith("gpsauto ")) {
        // Continuous GPS: "gpsauto <hex_id> <minutes> <seconds>"
        int firstSpace = command.indexOf(' ', 8);
        int secondSpace = command.indexOf(' ', firstSpace + 1);
        
        if (firstSpace > 0) {
            String idStr = command.substring(8, firstSpace);
            uint32_t targetID = strtoul(idStr.c_str(), NULL, 16);
            
            String minutesStr, secondsStr;
            unsigned long minutes = 0, seconds = 0;
            
            if (secondSpace > 0) {
                // Three parameters: id, minutes, seconds
                minutesStr = command.substring(firstSpace + 1, secondSpace);
                secondsStr = command.substring(secondSpace + 1);
                minutes = minutesStr.toInt();
                seconds = secondsStr.toInt();
            } else {
                // Two parameters: id, minutes (backwards compatibility)
                minutesStr = command.substring(firstSpace + 1);
                minutes = minutesStr.toInt();
                seconds = 0;
            }
            
            // Validate: interval = minutes * 60 + seconds
            unsigned long totalSeconds = (minutes * 60) + seconds;
            if (targetID > 0 && totalSeconds >= 1 && totalSeconds <= 86400) { // Max 24 hours
                gpsState.continuousMode = true;
                gpsState.targetID = targetID;
                gpsState.intervalMinutes = minutes;
                gpsState.intervalSeconds = seconds;
                gpsState.lastTransmission = 0; // Send immediately on next check
                
                stream->print("📍 Auto-GPS enabled: 0x");
                stream->print(targetID, HEX);
                stream->print(" every ");
                stream->print(minutes);
                stream->print("m ");
                stream->print(seconds);
                stream->println("s");
            } else {
                stream->println("❌ Invalid parameters. Use: gpsauto <hex_id> <0-1440_min> <0-59_sec>");
            }
        } else {
            stream->println("❌ Format: gpsauto <hex_id> <minutes> <seconds>");
        }
    }
    else if (command == "gpsstop") {
        gpsState.continuousMode = false;
        stream->println("📍 Auto-GPS transmission stopped");
    }
    else if (command == "gpsinfo") {
        stream->println("\\n📍 GPS Status:");
        stream->print("Current: ");
        stream->print(gpsState.latitude, 6);
        stream->print(", ");
        stream->println(gpsState.longitude, 6);
        stream->print("Valid Fix: ");
        stream->println(gpsState.hasValidFix ? "YES" : "NO");
        stream->print("GPS Time: ");
        stream->println(gpsState.hasValidTime ? "YES" : "NO");
        stream->print("Timestamp: ");
        stream->println(getGPSTimestamp());
        stream->print("Auto-send: ");
        stream->println(gpsState.continuousMode ? "ON" : "OFF");
        if (gpsState.continuousMode) {
            stream->print("Target: 0x");
            stream->println(gpsState.targetID, HEX);
            stream->print("Interval: ");
            stream->print(gpsState.intervalMinutes);
            stream->println(" minutes");
        }
        
        // Show raw GPS data for 2 seconds
        stream->println("\\n📡 Raw GPS Data (2 seconds):");
        stream->println("=============================");
        unsigned long startTime = millis();
        while (millis() - startTime < 2000) { // 2 seconds
            if (Serial.available()) {
                String rawData = Serial.readStringUntil('\n');
                rawData.trim();
                if (rawData.length() > 0) {
                    stream->println(rawData);
                }
            }
            delay(10); // Small delay to prevent blocking
        }
        stream->println("=============================");
    }
}
