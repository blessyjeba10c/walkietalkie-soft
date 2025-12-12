#include "MessageOverlay.h"
#include "DisplayManager.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern DisplayState displayState;

void addMessageToQueue(String message) {
    // Add message to circular queue
    if (displayState.messageQueueCount < 10) {
        displayState.messageQueue[displayState.messageQueueHead] = message;
        displayState.messageQueueHead = (displayState.messageQueueHead + 1) % 10;
        displayState.messageQueueCount++;
        
        // Activate overlay if not already active
        if (!displayState.messageOverlayActive) {
            displayState.messageOverlayActive = true;
            showMessageOverlay();
        }
    } else {
        // Queue full - overwrite oldest message
        displayState.messageQueue[displayState.messageQueueHead] = message;
        displayState.messageQueueHead = (displayState.messageQueueHead + 1) % 10;
        displayState.messageQueueTail = (displayState.messageQueueTail + 1) % 10;
    }
}

void showMessageOverlay() {
    if (!displayState.messageOverlayActive || displayState.messageQueueCount == 0) {
        return;
    }
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Draw border for overlay
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.drawBox(0, 0, 128, 12);
    
    // Header (inverted)
    u8g2.setDrawColor(0); // White on black
    char header[32];
    snprintf(header, sizeof(header), "MESSAGE (%d)", displayState.messageQueueCount);
    u8g2.drawStr(4, 9, header);
    u8g2.setDrawColor(1); // Black on white
    
    // Get current message from tail (oldest first)
    String currentMsg = displayState.messageQueue[displayState.messageQueueTail];
    
    // Word wrap the message (max 18 chars per line, 4 lines)
    int y = 24;
    int lineHeight = 10;
    int maxLines = 4;
    int lineCount = 0;
    
    // Simple word wrap
    int startPos = 0;
    while (startPos < currentMsg.length() && lineCount < maxLines) {
        int endPos = startPos + 18; // ~18 chars per line
        if (endPos >= currentMsg.length()) {
            endPos = currentMsg.length();
        } else {
            // Try to break at space
            int spacePos = currentMsg.lastIndexOf(' ', endPos);
            if (spacePos > startPos) {
                endPos = spacePos + 1;
            }
        }
        
        String line = currentMsg.substring(startPos, endPos);
        u8g2.drawStr(4, y, line.c_str());
        
        startPos = endPos;
        y += lineHeight;
        lineCount++;
    }
    
    // Footer - instructions
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(4, 60, "Press OK to dismiss");
    
    u8g2.sendBuffer();
}

void dismissCurrentMessage() {
    if (displayState.messageQueueCount > 0) {
        // Remove message from tail (FIFO)
        displayState.messageQueueTail = (displayState.messageQueueTail + 1) % 10;
        displayState.messageQueueCount--;
        
        if (displayState.messageQueueCount > 0) {
            // Show next message
            showMessageOverlay();
        } else {
            // No more messages - deactivate overlay
            displayState.messageOverlayActive = false;
            // Return to whatever screen was showing before
            updateDisplay();
        }
    }
}

bool hasMessagesInQueue() {
    return displayState.messageQueueCount > 0;
}
