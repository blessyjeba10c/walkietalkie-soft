#include "../DisplayManager.h"
#include "../GPSManager.h"
#include "../GSMManager.h"
#include "WalkieTalkie.h"

// Core display state and u8g2 object are defined in DisplayManager.cpp
extern DisplayState displayState;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

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
    
    delay(2000);
    
    // Initialize menu system
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
    delay(duration);
    
    // Return to main screen after showing message
    if (!displayState.inMenu) {
        showMainScreen();
    }
}

void displayError(String error) {
    showMessage("ERROR: " + error, 3000);
}

void displaySuccess(String success) {
    showMessage("OK: " + success, 2000);
}
