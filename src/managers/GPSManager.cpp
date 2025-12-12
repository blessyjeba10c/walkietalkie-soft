#include "GPSManager.h"
#include "DMR828S.h"
#include "WalkieTalkie.h"
#include "BluetoothSerial.h"

extern DMR828S dmr;
extern WalkieTalkieState wtState;
extern BluetoothSerial SerialBT;

GPSState gpsState;
TinyGPSPlus gps;

void initializeGPS() {
    // GPS module on Serial0 (9600 baud is standard for most GPS modules)
    Serial.begin(9600); // GPS module baud rate
    delay(1000);
    SerialBT.println("📍 GPS module initialized (TinyGPS++)");
}

void readGPS() {
    // Feed data from Serial to TinyGPS++
    while (Serial.available()) {
        char c = Serial.read();
        gps.encode(c);
    }
    
    // Update GPS state from TinyGPS++ data
    if (gps.location.isUpdated()) {
        if (gps.location.isValid()) {
            gpsState.latitude = gps.location.lat();
            gpsState.longitude = gps.location.lng();
            gpsState.lastLatitude = gpsState.latitude;
            gpsState.lastLongitude = gpsState.longitude;
            gpsState.hasValidFix = true;
            gpsState.hasLastLocation = true;
        } else {
            gpsState.hasValidFix = false;
        }
    }
    
    // Update time data
    if (gps.time.isUpdated() && gps.time.isValid()) {
        gpsState.gpsHour = gps.time.hour();
        gpsState.gpsMinute = gps.time.minute();
        gpsState.gpsSecond = gps.time.second();
        gpsState.hasValidTime = true;
    }
    
    // Update date data
    if (gps.date.isUpdated() && gps.date.isValid()) {
        gpsState.gpsDay = gps.date.day();
        gpsState.gpsMonth = gps.date.month();
        gpsState.gpsYear = gps.date.year();
    }
    
    // Update GPS read timestamp
    gpsState.lastGPSRead = millis();
}

void parseNMEA(String sentence) {
    // Legacy function - now handled by TinyGPS++
    // Feed the sentence to TinyGPS++
    for (unsigned int i = 0; i < sentence.length(); i++) {
        gps.encode(sentence.charAt(i));
    }
}

void sendGPSLocation(Stream* stream, uint32_t targetID) {
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
        lat = 29.938971327453903;
        lon = 77.56449807342506;
        status = "DEFAULT";
    }
    
    // Create GPS message
    String gpsMessage = "GPS " + status + ": ";
    gpsMessage += String(lat, 6) + "," + String(lon, 6);
    
    // Send via SMS - need to access DMR instance from main
    // This will be handled by the calling code
    stream->print("📍 GPS ready to send to 0x");
    stream->print(targetID, HEX);
    stream->print(" (");
    stream->print(status);
    stream->print("): ");
    stream->print(lat, 6);
    stream->print(", ");
    stream->println(lon, 6);
}

void handleContinuousGPS() {
    if (!gpsState.continuousMode) return;
    
    unsigned long currentTime = millis();
    unsigned long totalSeconds = (gpsState.intervalMinutes * 60) + gpsState.intervalSeconds;
    unsigned long intervalMs = totalSeconds * 1000; // Convert to milliseconds
    
    // Check if it's time to send GPS
    if (gpsState.lastTransmission == 0 || 
        (currentTime - gpsState.lastTransmission) >= intervalMs) {
        
        gpsState.lastTransmission = currentTime;
        
        // Get GPS data
        double lat, lon;
        String status;
        String gpsMessage;
        
        if (gpsState.hasValidFix) {
            // Current GPS fix available
            lat = gpsState.latitude;
            lon = gpsState.longitude;
            status = "CURRENT";
            gpsMessage = "GPS " + status + ": ";
            gpsMessage += wtState.soldierID + ",";
            gpsMessage += String(lat, 6) + "," + String(lon, 6);
        } else if (gpsState.hasLastLocation) {
            // Use last known location
            lat = gpsState.lastLatitude;
            lon = gpsState.lastLongitude;
            status = "LAST GPS";
            gpsMessage = "GPS " + status + ": ";
            gpsMessage += wtState.soldierID + ",";
            gpsMessage += String(lat, 6) + "," + String(lon, 6);
        } else {
            // No GPS fix available - use default location
            lat = 29.863594615284676;
            lon = 77.89650459615501;
            status = "DEFAULT";
            gpsMessage = "GPS " + status + ": ";
            gpsMessage += wtState.soldierID + ",";
            gpsMessage += String(lat, 6) + "," + String(lon, 6);
            status = "NO FIX";
            gpsMessage = "GPS NO FIX: " + wtState.soldierID + ",waiting for signal";
        }
        
        // Send via SMS
        if (dmr.sendSMS(gpsState.targetID, gpsMessage.c_str())) {
            SerialBT.print("📍 Auto-GPS sent to 0x");
            SerialBT.print(gpsState.targetID, HEX);
            SerialBT.print(" (");
            SerialBT.print(status);
            SerialBT.print(")");
            if (status != "NO FIX") {
                SerialBT.print(": ");
                SerialBT.print(lat, 6);
                SerialBT.print(", ");
                SerialBT.print(lon, 6);
            }
            SerialBT.println();
        } else {
            SerialBT.println("❌ Auto-GPS transmission failed");
        }
    }
}

String getGPSTimestamp() {
    if (gpsState.hasValidTime) {
        // Format: YYYY-MM-DDTHH:MM:SSZ
        String timestamp = String(gpsState.gpsYear) + "-";
        
        if (gpsState.gpsMonth < 10) timestamp += "0";
        timestamp += String(gpsState.gpsMonth) + "-";
        
        if (gpsState.gpsDay < 10) timestamp += "0";
        timestamp += String(gpsState.gpsDay) + "T";
        
        if (gpsState.gpsHour < 10) timestamp += "0";
        timestamp += String(gpsState.gpsHour) + ":";
        
        if (gpsState.gpsMinute < 10) timestamp += "0";
        timestamp += String(gpsState.gpsMinute) + ":";
        
        if (gpsState.gpsSecond < 10) timestamp += "0";
        timestamp += String(gpsState.gpsSecond) + "Z";
        
        return timestamp;
    } else {
        // Fallback to system time if GPS time not available
        String timestamp = "2025-11-20T";
        
        unsigned long currentTime = millis() / 1000;
        int hours = (currentTime / 3600) % 24;
        int minutes = (currentTime / 60) % 60;
        int seconds = currentTime % 60;
        
        if (hours < 10) timestamp += "0";
        timestamp += String(hours) + ":";
        if (minutes < 10) timestamp += "0";
        timestamp += String(minutes) + ":";
        if (seconds < 10) timestamp += "0";
        timestamp += String(seconds) + "Z";
        
        return timestamp;
    }
}