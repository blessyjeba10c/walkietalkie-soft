#include "DisplayManager.h"
#include "GPSManager.h"
#include "GSMManager.h"
#include "WalkieTalkie.h"

extern DisplayState displayState;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

void showMainScreen() {
    u8g2.clearBuffer();
    
    // Top status bar - signal indicators
    u8g2.setFont(u8g2_font_5x7_tf);
    
    // DMR signal icon (left)
    u8g2.drawStr(0, 7, "DMR");
    // Draw signal bars (always show connected)
    for (int i = 0; i < 4; i++) {
        u8g2.drawVLine(20 + i*3, 7 - i, i + 1);
    }
    
    // GPS signal (middle)
    if (gpsState.hasValidFix) {
        u8g2.drawStr(40, 7, "GPS");
        // Draw satellite icon bars
        for (int i = 0; i < 3; i++) {
            u8g2.drawVLine(55 + i*3, 7 - i, i + 1);
        }
    } else {
        u8g2.drawStr(40, 7, "GPS?");
    }
    
    // GSM signal (right)
    if (gsmState.networkRegistered) {
        u8g2.drawStr(75, 7, "GSM");
        // Signal strength bars (0-5 bars based on signal)
        int bars = map(gsmState.signalStrength, 0, 31, 0, 5);
        for (int i = 0; i < bars && i < 5; i++) {
            u8g2.drawVLine(95 + i*3, 7 - i, i + 1);
        }
    } else {
        u8g2.drawStr(75, 7, "---");
    }
    
    // Divider line
    u8g2.drawHLine(0, 9, 128);
    
    // Main frequency display
    u8g2.setFont(u8g2_font_8x13B_tf);
    char freqLine[20];
    snprintf(freqLine, sizeof(freqLine), "143.000 MHz");
    int freqWidth = u8g2.getStrWidth(freqLine);
    u8g2.drawStr((128 - freqWidth) / 2, 28, freqLine);
    
    // Divider
    u8g2.drawHLine(0, 32, 128);
    
    // Bottom navigation menu
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(8, 50, "#Menu");
    u8g2.drawStr(70, 50, "#back");
    
    // Draw vertical separator
    u8g2.drawVLine(64, 35, 20);
    
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
