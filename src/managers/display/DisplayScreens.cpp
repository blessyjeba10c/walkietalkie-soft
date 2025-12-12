#include "../DisplayManager.h"
#include "../GPSManager.h"
#include "../GSMManager.h"
#include "WalkieTalkie.h"

extern DisplayState displayState;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

void showMainScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Title
    u8g2.drawStr(0, 10, "DMR Radio System");
    u8g2.drawHLine(0, 12, 128);
    
    // Status indicators
    char statusLine[32];
    snprintf(statusLine, sizeof(statusLine), "CH:%d VOL:%d", wtState.currentChannel, wtState.volume);
    u8g2.drawStr(0, 25, statusLine);
    
    // GPS Status
    if (gpsState.hasValidFix) {
        u8g2.drawStr(0, 35, "GPS: LOCK");
    } else {
        u8g2.drawStr(0, 35, "GPS: SEARCH");
    }
    
    // GSM Status  
    char gsmLine[32];
    if (gsmState.networkRegistered) {
        snprintf(gsmLine, sizeof(gsmLine), "GSM: REG %d", gsmState.signalStrength);
    } else {
        snprintf(gsmLine, sizeof(gsmLine), "GSM: NO NET");
    }
    u8g2.drawStr(0, 45, gsmLine);
    
    // Menu options
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(0, 60, "*=Menu #=Back A=Call");
    
    u8g2.sendBuffer();
}

void showStatusScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    u8g2.drawStr(0, 10, "System Status");
    u8g2.drawHLine(0, 12, 128);
    
    char line[32];
    snprintf(line, sizeof(line), "Radio ID: 0x%X", (unsigned int)wtState.myRadioID);
    u8g2.drawStr(0, 25, line);
    
    snprintf(line, sizeof(line), "Channel: %d", wtState.currentChannel);
    u8g2.drawStr(0, 35, line);
    
    snprintf(line, sizeof(line), "GPS Fix: %s", gpsState.hasValidFix ? "YES" : "NO");
    u8g2.drawStr(0, 45, line);
    
    snprintf(line, sizeof(line), "GSM Reg: %s", gsmState.networkRegistered ? "YES" : "NO");
    u8g2.drawStr(0, 55, line);
    
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(0, 64, "#=Back");
    
    u8g2.sendBuffer();
}

void showGPSScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    u8g2.drawStr(0, 10, "GPS Information");
    u8g2.drawHLine(0, 12, 128);
    
    if (gpsState.hasValidFix) {
        u8g2.drawStr(0, 25, "Status: LOCKED");
        
        char latLine[32];
        snprintf(latLine, sizeof(latLine), "Lat: %.6f", gpsState.latitude);
        u8g2.drawStr(0, 35, latLine);
        
        char lonLine[32]; 
        snprintf(lonLine, sizeof(lonLine), "Lon: %.6f", gpsState.longitude);
        u8g2.drawStr(0, 45, lonLine);
    } else {
        u8g2.drawStr(0, 25, "Status: SEARCHING");
        u8g2.drawStr(0, 35, "Satellites: --");
        u8g2.drawStr(0, 45, "Waiting for fix...");
    }
    
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(0, 64, "#=Back C=Send");
    
    u8g2.sendBuffer();
}

void showGSMScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    u8g2.drawStr(0, 10, "GSM Information");
    u8g2.drawHLine(0, 12, 128);
    
    char line[32];
    snprintf(line, sizeof(line), "Status: %s", gsmState.networkRegistered ? "REG" : "NO REG");
    u8g2.drawStr(0, 25, line);
    
    snprintf(line, sizeof(line), "Signal: %d/31", gsmState.signalStrength);
    u8g2.drawStr(0, 35, line);
    
    if (gsmState.phoneNumber.length() > 0) {
        snprintf(line, sizeof(line), "Phone: %s", gsmState.phoneNumber.c_str());
        u8g2.drawStr(0, 45, line);
    } else {
        u8g2.drawStr(0, 45, "Phone: Not set");
    }
    
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(0, 64, "#=Back B=SMS");
    
    u8g2.sendBuffer();
}

void showMessageHistory() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tf);
    
    u8g2.drawStr(0, 8, "Message History");
    u8g2.drawHLine(0, 10, 128);
    
    // Display last 6 messages
    int y = 18;
    int lineHeight = 9;
    int maxCharsPerLine = 21;
    
    // Start from the most recent message
    for (int i = 0; i < 6; i++) {
        // Calculate the index to display messages in reverse chronological order
        int msgIndex = (displayState.messageIndex - 1 - i + 6) % 6;
        
        if (displayState.messages[msgIndex].length() > 0) {
            String msg = displayState.messages[msgIndex];
            
            // Truncate if too long
            if (msg.length() > maxCharsPerLine) {
                msg = msg.substring(0, maxCharsPerLine - 3) + "...";
            }
            
            u8g2.drawStr(0, y, msg.c_str());
            y += lineHeight;
        }
    }
    
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr(0, 62, "#=Back");
    
    u8g2.sendBuffer();
}
