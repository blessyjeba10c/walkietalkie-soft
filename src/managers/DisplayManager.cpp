#include "DisplayManager.h"
#include "MenuManager.h"
#include "InputManager.h"
#include "MessageOverlay.h"
#include "GPSManager.h"
#include "GSMManager.h"
#include "WalkieTalkie.h"

// Global display state and U8g2 object
DisplayState displayState;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

void initializeDisplay() {
    // Initialize U8g2
    u8g2.begin();
    u8g2.enableUTF8Print();
    
    displayState.initialized = true;
    
    // Show startup screen
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "DMR Walkie-Talkie");
    u8g2.drawHLine(0, 12, 128);
    u8g2.drawStr(0, 25, "GPS: Initializing...");
    u8g2.drawStr(0, 35, "GSM: Initializing...");
    u8g2.drawStr(0, 45, "DMR: Initializing...");
    u8g2.drawStr(0, 60, "Use keypad to navigate");
    u8g2.sendBuffer();
    
    // Initialize menu system immediately - no delay needed
    initializeMenus();
    showMenu();
}

void updateDisplay() {
    if (!displayState.initialized) return;
    
    // Update display every 500ms or when needed
    if (millis() - displayState.lastUpdate > 500) {
        displayState.lastUpdate = millis();
        
        // Message overlay has highest priority
        if (displayState.messageOverlayActive) {
            showMessageOverlay();
        } else if (displayState.inputMode) {
            showInputScreen();
        } else if (displayState.inMenu) {
            showMenu();
        } else if (displayState.currentScreen == "main") {
            showMainScreen();
        } else if (displayState.currentScreen == "status") {
            showStatusScreen();
        } else if (displayState.currentScreen == "gps") {
            showGPSScreen();
        } else if (displayState.currentScreen == "gsm") {
            showGSMScreen();
        } else if (displayState.currentScreen == "messages") {
            showMessageHistory();
        }
    }
}

void showMainScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Title - Home Screen
    u8g2.drawStr(0, 10, "Home Screen");
    u8g2.drawHLine(0, 12, 128);
    
    // Line 1: GSM/GPS/LoRa Status
    u8g2.setFont(u8g2_font_5x7_tf);
    char line1[32];
    snprintf(line1, sizeof(line1), "GSM:%s GPS:%s LoRa:%s",
        gsmState.networkRegistered ? "Y" : "N",
        gpsState.hasValidFix ? "Y" : "N",
        "Y");
    u8g2.drawStr(0, 22, line1);
    
    // Line 2: GSM Phone
    char line2[32];
    if (gsmState.phoneNumber.length() > 0) {
        snprintf(line2, sizeof(line2), "GSM:%s", gsmState.phoneNumber.substring(0, 12).c_str());
    } else {
        snprintf(line2, sizeof(line2), "GSM: Not set");
    }
    u8g2.drawStr(0, 32, line2);
    
    // Line 3: Frequency (placeholder - 142.000 MHz)
    u8g2.drawStr(0, 42, "142.000 MHz");
    
    // Line 4: Channel and Radio ID
    char line4[32];
    snprintf(line4, sizeof(line4), "CH:%d ID:0x%X", 
        wtState.currentChannel, 
        (unsigned int)wtState.myRadioID);
    u8g2.drawStr(0, 52, line4);
    
    // Bottom: Send Mode and Menu hint
    u8g2.setFont(u8g2_font_4x6_tf);
    char bottomLine[32];
    snprintf(bottomLine, sizeof(bottomLine), "Mode:%s *=Menu", wtState.currentSendMode.c_str());
    u8g2.drawStr(0, 62, bottomLine);
    
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
        u8g2.drawStr(0, 25, "Status: DEFAULT");
        
        char latLine[32];
        snprintf(latLine, sizeof(latLine), "Lat: 29.863595");
        u8g2.drawStr(0, 35, latLine);
        
        char lonLine[32]; 
        snprintf(lonLine, sizeof(lonLine), "Lon: 77.896505");
        u8g2.drawStr(0, 45, lonLine);
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

void addMessage(String message) {
    displayState.messages[displayState.messageIndex] = message;
    displayState.messageIndex = (displayState.messageIndex + 1) % 6;
}

void showMessage(String message, int duration) {
    if (!displayState.initialized) return;
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tf);  // Use smaller font for more text
    
    // Clean up message - remove emojis and excessive formatting
    message.replace("📤", ">");
    message.replace("📞", ">");
    message.replace("📻", ">");
    message.replace("🔊", ">");
    message.replace("🆔", ">");
    message.replace("🔒", "");
    message.replace("🔓", "");
    message.replace("❌", "X");
    message.replace("✅", "OK");
    message.replace("🔐", "");
    message.replace("📊", "");
    message.replace("📶", "");
    message.replace("📍", "");
    message.replace("📱", "");
    message.replace("🔧", "");
    message.replace("📖", "");
    message.replace("📡", "");
    message.replace("🚨", "!");
    message.replace("🔄", "");
    
    // Split message into lines that fit the screen
    int maxCharsPerLine = 21; // 128px / 6px per char ≈ 21 chars
    int maxLines = 8; // 64px / 8px per line ≈ 8 lines
    String lines[8];
    int lineCount = 0;
    
    int startPos = 0;
    while (startPos < message.length() && lineCount < maxLines) {
        // Find natural break point (newline or word boundary)
        int endPos = startPos + maxCharsPerLine;
        if (endPos >= message.length()) {
            endPos = message.length();
        } else {
            // Try to break at word boundary
            int lastSpace = message.lastIndexOf(' ', endPos);
            int newlinePos = message.indexOf('\n', startPos);
            
            if (newlinePos != -1 && newlinePos < endPos) {
                endPos = newlinePos;
            } else if (lastSpace > startPos && lastSpace < endPos) {
                endPos = lastSpace;
            }
        }
        
        String line = message.substring(startPos, endPos);
        line.trim();
        
        if (line.length() > 0) {
            lines[lineCount] = line;
            lineCount++;
        }
        
        startPos = endPos + 1;
    }
    
    // Display lines with proper spacing
    int lineHeight = 8;
    int startY = 8;
    
    for (int i = 0; i < lineCount; i++) {
        u8g2.drawStr(0, startY + (i * lineHeight), lines[i].c_str());
    }
    
    // Add scroll indicator if message was truncated
    if (startPos < message.length()) {
        u8g2.drawStr(115, 62, "...");
    }
    
    // Add back indicator
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr(0, 62, "#=Back");
    
    u8g2.sendBuffer();
    // Note: Delay removed to prevent watchdog resets
    // Message will stay on screen until next update cycle
}

void displayError(String error) {
    showMessage("ERROR: " + error, 3000);
}

void displaySuccess(String success) {
    showMessage("OK: " + success, 2000);
}
