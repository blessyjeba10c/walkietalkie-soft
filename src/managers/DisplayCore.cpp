#include "DisplayManager.h"
#include "GPSManager.h"
#include "GSMManager.h"
#include "WalkieTalkie.h"

// Global instances
DisplayState displayState;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

void initializeDisplay() {
    // Initialize U8g2
    u8g2.begin();
    u8g2.enableUTF8Print();
    
    displayState.initialized = true;
    
    // Show startup screen
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13B_tf);
    u8g2.drawStr(10, 20, "WALKIE-TALKIE");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawHLine(0, 25, 128);
    u8g2.drawStr(20, 38, "DMR + GPS + GSM");
    u8g2.drawStr(25, 50, "+ LoRa Mesh");
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(25, 62, "Initializing...");
    u8g2.sendBuffer();
    
    delay(2000);
    
    // Initialize menu system
    initializeMenus();
    displayState.inMenu = false;
    showMainScreen();
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
    // Add message to circular buffer
    displayState.messages[displayState.messageIndex] = message;
    displayState.messageIndex = (displayState.messageIndex + 1) % 6;
    
    // Add to message queue for overlay
    if (displayState.messageQueueCount < 10) {
        displayState.messageQueue[displayState.messageQueueHead] = message;
        displayState.messageQueueHead = (displayState.messageQueueHead + 1) % 10;
        displayState.messageQueueCount++;
        displayState.messageOverlayActive = true;
    }
}

void showMessage(String message, int duration) {
    if (!displayState.initialized) return;
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tf);
    
    // Clean up message - remove emojis
    message.replace("📤", ">");
    message.replace("📞", ">");
    message.replace("📻", ">");
    message.replace("🔊", ">");
    message.replace("🆔", ">");
    message.replace("❌", "X");
    message.replace("✅", "OK");
    message.replace("🚨", "!");
    
    // Split message into lines
    int maxCharsPerLine = 21;
    int maxLines = 8;
    String lines[8];
    int lineCount = 0;
    
    int startPos = 0;
    while (startPos < message.length() && lineCount < maxLines) {
        int endPos = startPos + maxCharsPerLine;
        if (endPos >= message.length()) {
            endPos = message.length();
        } else {
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
    
    // Display lines
    int lineHeight = 8;
    int startY = 8;
    
    for (int i = 0; i < lineCount; i++) {
        u8g2.drawStr(0, startY + (i * lineHeight), lines[i].c_str());
    }
    
    // Add scroll indicator if truncated
    if (startPos < message.length()) {
        u8g2.drawStr(115, 62, "...");
    }
    
    // Add back indicator
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr(0, 62, "#=Back");
    
    u8g2.sendBuffer();
    delay(duration);
    
    // Return to main screen
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

