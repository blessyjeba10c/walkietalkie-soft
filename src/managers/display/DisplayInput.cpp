#include "../DisplayManager.h"
#include "CommandProcessor.h"
#include "WalkieTalkie.h"

extern DisplayState displayState;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

// Output capture system
class CaptureStream : public Stream {
public:
    String capturedText = "";
    
    // Required Stream methods
    int available() override { return 0; }
    int read() override { return -1; }
    int peek() override { return -1; }
    void flush() override {}
    
    // Print methods to capture output
    size_t write(uint8_t c) override {
        if (c == '\r') return 1; // Skip carriage returns
        capturedText += (char)c;
        return 1;
    }
    
    size_t write(const uint8_t *buffer, size_t size) override {
        for (size_t i = 0; i < size; i++) {
            write(buffer[i]);
        }
        return size;
    }
    
    void clear() {
        capturedText = "";
    }
};

CaptureStream captureStream;

void captureCommandOutput(String command) {
    captureStream.clear();
    processCommand(&captureStream, command);
    displayState.lastCommandOutput = captureStream.capturedText;
}

void displayCapturedOutput() {
    if (displayState.lastCommandOutput.length() > 0) {
        showMessage(displayState.lastCommandOutput, 5000);
        displayState.lastCommandOutput = "";
    }
}

void executeMenuAction(String action) {
    // Handle input actions
    if (action.startsWith("input_")) {
        if (action == "input_call") {
            startInput("Enter Radio ID:", "call ");
        } else if (action == "input_group") {
            startInput("Enter Group ID:", "group ");
        } else if (action == "input_radioid") {
            startInput("Enter Radio ID:", "radioid ");
        } else if (action == "input_channel") {
            startInput("Enter Channel:", "channel ");
        } else if (action == "input_volume") {
            startInput("Enter Volume:", "volume ");
        } else if (action == "input_encryptkey") {
            startInput("Enter Key (32 hex):", "encryptkey ");
        } else if (action == "input_sms") {
            String prompts[] = {"Enter Radio ID:", "Enter Message:"};
            startMultiStepInput("sms ", prompts, 2);
        } else if (action == "input_gsmsms") {
            String prompts[] = {"Enter Phone #:", "Enter Message:"};
            startMultiStepInput("gsmsms ", prompts, 2);
        } else if (action == "input_gsmphone") {
            startInput("Enter Phone #:", "gsmphone ");
        } else if (action == "input_raw") {
            startInput("Enter Raw CMD:", "raw ");
        } else if (action == "input_gps") {
            startInput("Enter Radio ID:", "gps ");
        } else if (action == "input_gpsauto") {
            String prompts[] = {"Enter Radio ID:", "Enter Minutes:", "Enter Seconds:"};
            startMultiStepInput("gpsauto ", prompts, 3);
        } else if (action == "input_soldierid") {
            startInput("Enter Soldier ID:", "soldierid ");
        } else if (action == "input_lorasms") {
            String prompts[] = {"Enter Message:", "Include ID? (Y/N):"};
            startMultiStepInput("lorasms_input ", prompts, 2);
        } else if (action == "input_smartsend") {
            startInput("Enter Message:", "smartsend ");
        }
        return;
    }
    
    // Handle direct commands
    if (action == "exit_menu") {
        displayState.inMenu = false;
        showMainScreen();
    } else if (action == "show_messages") {
        displayState.inMenu = false;
        displayState.currentScreen = "messages";
        showMessageHistory();
    } else if (action == "back") {
        goBack();
    } else if (action == "send_position") {
        startInput("Enter Radio ID:", "gps ");
    } else if (action == "emergency") {
        processCommand(&SerialBT, "emergency");
        displaySuccess("Emergency signal sent");
    } else if (action == "stop") {
        processCommand(&SerialBT, "stop");
        displaySuccess("Call stopped");
    } else if (action == "fallback") {
        processCommand(&SerialBT, "fallback");
        displaySuccess("Fallback mode activated");
    } else if (action == "encrypt on") {
        processCommand(&SerialBT, "encrypt on");
        displaySuccess("Encryption enabled");
    } else if (action == "encrypt off") {
        processCommand(&SerialBT, "encrypt off");
        displaySuccess("Encryption disabled");
    } else if (action == "encrypt status") {
        captureCommandOutput("encrypt status");
        processCommand(&SerialBT, "encrypt status");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "status") {
        captureCommandOutput("status");
        processCommand(&SerialBT, "status");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "info") {
        captureCommandOutput("info");
        processCommand(&SerialBT, "info");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "help") {
        captureCommandOutput("help");
        processCommand(&SerialBT, "help");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "gsmstatus") {
        captureCommandOutput("gsmstatus");
        processCommand(&SerialBT, "gsmstatus");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "smsinfo") {
        captureCommandOutput("smsinfo");
        processCommand(&SerialBT, "smsinfo");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "bt") {
        captureCommandOutput("bt");
        processCommand(&SerialBT, "bt");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "lorastatus") {
        captureCommandOutput("lorastatus");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "i2cscan") {
        captureCommandOutput("i2cscan");
        processCommand(&SerialBT, "i2cscan");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "keytest") {
        captureCommandOutput("keytest");
        processCommand(&SerialBT, "keytest");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "keyscan") {
        captureCommandOutput("keyscan");
        processCommand(&SerialBT, "keyscan");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "gps_position") {
        captureCommandOutput("gpsinfo");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "gps_satellites") {
        captureCommandOutput("gpsinfo");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "gps_reset" || action == "gpsstop") {
        captureCommandOutput("gpsstop");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "gsm_signal") {
        captureCommandOutput("gsmstatus");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "system_info") {
        captureCommandOutput("info");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "display_settings") {
        captureCommandOutput("info");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "radio_settings") {
        captureCommandOutput("status");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "debug_mode") {
        displaySuccess("Debug mode toggled");
        displayState.inMenu = false;
        delay(500);
    }
}

void startInput(String prompt, String action) {
    displayState.inputMode = true;
    displayState.inputPrompt = prompt;
    displayState.inputValue = "";
    displayState.pendingAction = action;
    showInputScreen();
}

void handleInput(char c) {
    if (!displayState.inputMode) return;
    
    // Use multi-step input handler if in multi-step mode
    if (displayState.multiStepInput) {
        handleMultiStepInput(c);
        return;
    }
    
    if (c == '#') {
        // Cancel input
        cancelInput();
    } else if (c == '*') {
        // Confirm input
        confirmInput();
    } else if (c == 'C') {
        // Backspace
        if (displayState.inputValue.length() > 0) {
            displayState.inputValue.remove(displayState.inputValue.length() - 1);
            showInputScreen();
        }
    } else if (c != 0) {
        // Add character
        displayState.inputValue += c;
        showInputScreen();
    }
}

void cancelInput() {
    displayState.inputMode = false;
    showMenu();
}

void confirmInput() {
    if (displayState.inputValue.length() > 0) {
        String fullCommand = displayState.pendingAction + displayState.inputValue;
        captureCommandOutput(fullCommand);
        processCommand(&SerialBT, fullCommand);
        displayState.inputMode = false;
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else {
        displayState.inputMode = false;
        showMenu();
    }
}

void showInputScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Title with step info for multi-step input
    if (displayState.multiStepInput) {
        String title = "Step " + String(displayState.inputStep + 1);
        u8g2.drawStr(0, 10, title.c_str());
        
        // Show previous step values
        if (displayState.inputStep > 0) {
            u8g2.setFont(u8g2_font_4x6_tf);
            u8g2.drawStr(80, 10, displayState.stepValues[displayState.inputStep - 1].c_str());
            u8g2.setFont(u8g2_font_6x10_tf);
        }
    } else {
        u8g2.drawStr(0, 10, "Input Required");
    }
    u8g2.drawHLine(0, 12, 128);
    
    // Prompt
    u8g2.drawStr(0, 25, displayState.inputPrompt.c_str());
    
    // Input value with cursor
    String displayValue = displayState.inputValue + "_";
    u8g2.drawStr(0, 40, displayValue.c_str());
    
    // Help text
    u8g2.setFont(u8g2_font_4x6_tf);
    if (displayState.multiStepInput) {
        u8g2.drawStr(0, 55, "Enter value, * for next step");
    } else {
        u8g2.drawStr(0, 55, "Enter value, * to confirm");
    }
    u8g2.drawStr(0, 62, "# to cancel, C to backspace");
    
    u8g2.sendBuffer();
}

void startMultiStepInput(String action, String prompts[], int stepCount) {
    displayState.multiStepInput = true;
    displayState.inputMode = true;
    displayState.inputStep = 0;
    displayState.pendingAction = action;
    
    // Copy prompts to display state
    for (int i = 0; i < stepCount && i < 3; i++) {
        displayState.stepPrompts[i] = prompts[i];
        displayState.stepValues[i] = "";
    }
    
    // Start with first prompt
    displayState.inputPrompt = displayState.stepPrompts[0];
    displayState.inputValue = "";
    showInputScreen();
}

void handleMultiStepInput(char c) {
    if (!displayState.inputMode || !displayState.multiStepInput) return;
    
    if (c == '#') {
        // Cancel multi-step input
        displayState.multiStepInput = false;
        displayState.inputStep = 0;
        cancelInput();
    } else if (c == '*') {
        // Confirm current step
        if (displayState.inputValue.length() > 0) {
            // Save current step value
            displayState.stepValues[displayState.inputStep] = displayState.inputValue;
            nextInputStep();
        }
    } else if (c == 'C') {
        // Backspace
        if (displayState.inputValue.length() > 0) {
            displayState.inputValue.remove(displayState.inputValue.length() - 1);
            showInputScreen();
        }
    } else if (c != 0) {
        // Add character
        displayState.inputValue += c;
        showInputScreen();
    }
}

void nextInputStep() {
    displayState.inputStep++;
    
    // Check if we need more steps
    if (displayState.inputStep < 3 && displayState.stepPrompts[displayState.inputStep].length() > 0) {
        // Move to next step
        displayState.inputPrompt = displayState.stepPrompts[displayState.inputStep];
        displayState.inputValue = "";
        showInputScreen();
    } else {
        // All steps completed
        completeMultiStepInput();
    }
}

void completeMultiStepInput() {
    // Build command from all step values
    String fullCommand = displayState.pendingAction;
    
    // Special handling for LoRa message with ID option
    if (displayState.pendingAction == "lorasms_input ") {
        String message = displayState.stepValues[0];
        String includeID = displayState.stepValues[1];
        includeID.toUpperCase();
        
        fullCommand = "lorasms " + message;
        if (includeID.startsWith("N")) {
            fullCommand += " --no-id";
        }
    } else {
        // Normal multi-step command building
        for (int i = 0; i < displayState.inputStep; i++) {
            if (i > 0) fullCommand += " ";
            fullCommand += displayState.stepValues[i];
        }
    }
    
    // Execute command
    captureCommandOutput(fullCommand);
    processCommand(&SerialBT, fullCommand);
    
    // Reset multi-step state
    displayState.multiStepInput = false;
    displayState.inputStep = 0;
    displayState.inputMode = false;
    
    // Show result
    displayCapturedOutput();
    displayState.inMenu = false;
    delay(500);
}

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
