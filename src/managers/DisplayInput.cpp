#include "DisplayManager.h"
#include "CommandProcessor.h"
#include "BluetoothSerial.h"

extern DisplayState displayState;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern BluetoothSerial SerialBT;

// Output capture stream
class CaptureStream : public Stream {
public:
    String capturedText = "";
    
    int available() override { return 0; }
    int read() override { return -1; }
    int peek() override { return -1; }
    void flush() override {}
    
    size_t write(uint8_t c) override {
        if (c == '\r') return 1;
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
    } else if (action == "ackon") {
        processCommand(&SerialBT, "ackon");
        displaySuccess("LoRa ACK enabled");
    } else if (action == "ackoff") {
        processCommand(&SerialBT, "ackoff");
        displaySuccess("LoRa ACK disabled");
    } else if (action == "send_position") {
        startInput("Enter Radio ID:", "gps ");
    } else if (action == "emergency") {
        processCommand(&SerialBT, "emergency");
        displaySuccess("Emergency sent");
    } else if (action == "stop") {
        processCommand(&SerialBT, "stop");
        displaySuccess("Call stopped");
    } else if (action == "fallback") {
        processCommand(&SerialBT, "fallback");
        displaySuccess("Fallback activated");
    } else if (action == "encrypt on") {
        processCommand(&SerialBT, "encrypt on");
        displaySuccess("Encryption ON");
    } else if (action == "encrypt off") {
        processCommand(&SerialBT, "encrypt off");
        displaySuccess("Encryption OFF");
    } else if (action.endsWith("status") || action.endsWith("info") || action.startsWith("gps")) {
        // Commands that show info
        captureCommandOutput(action);
        processCommand(&SerialBT, action);
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else {
        // Generic command
        processCommand(&SerialBT, action);
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
    
    if (displayState.multiStepInput) {
        handleMultiStepInput(c);
        return;
    }
    
    if (c == '#') {
        cancelInput();
    } else if (c == '*') {
        confirmInput();
    } else if (c == 'C') {
        if (displayState.inputValue.length() > 0) {
            displayState.inputValue.remove(displayState.inputValue.length() - 1);
            showInputScreen();
        }
    } else if (c != 0) {
        displayState.inputValue += c;
        showInputScreen();
    }
}

void cancelInput() {
    displayState.inputMode = false;
    displayState.multiStepInput = false;
    displayState.inputStep = 0;
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
    
    if (displayState.multiStepInput) {
        String title = "Step " + String(displayState.inputStep + 1);
        u8g2.drawStr(0, 10, title.c_str());
        
        if (displayState.inputStep > 0) {
            u8g2.setFont(u8g2_font_4x6_tf);
            u8g2.drawStr(80, 10, displayState.stepValues[displayState.inputStep - 1].c_str());
            u8g2.setFont(u8g2_font_6x10_tf);
        }
    } else {
        u8g2.drawStr(0, 10, "Input Required");
    }
    u8g2.drawHLine(0, 12, 128);
    
    u8g2.drawStr(0, 25, displayState.inputPrompt.c_str());
    
    String displayValue = displayState.inputValue + "_";
    u8g2.drawStr(0, 40, displayValue.c_str());
    
    u8g2.setFont(u8g2_font_4x6_tf);
    if (displayState.multiStepInput) {
        u8g2.drawStr(0, 55, "Enter value, * for next");
    } else {
        u8g2.drawStr(0, 55, "Enter value, * confirm");
    }
    u8g2.drawStr(0, 62, "# cancel, C backspace");
    
    u8g2.sendBuffer();
}

void startMultiStepInput(String action, String prompts[], int stepCount) {
    displayState.multiStepInput = true;
    displayState.inputMode = true;
    displayState.inputStep = 0;
    displayState.pendingAction = action;
    
    for (int i = 0; i < stepCount && i < 3; i++) {
        displayState.stepPrompts[i] = prompts[i];
        displayState.stepValues[i] = "";
    }
    
    displayState.inputPrompt = displayState.stepPrompts[0];
    displayState.inputValue = "";
    showInputScreen();
}

void handleMultiStepInput(char c) {
    if (!displayState.inputMode || !displayState.multiStepInput) return;
    
    if (c == '#') {
        cancelInput();
    } else if (c == '*') {
        if (displayState.inputValue.length() > 0) {
            displayState.stepValues[displayState.inputStep] = displayState.inputValue;
            nextInputStep();
        }
    } else if (c == 'C') {
        if (displayState.inputValue.length() > 0) {
            displayState.inputValue.remove(displayState.inputValue.length() - 1);
            showInputScreen();
        }
    } else if (c != 0) {
        displayState.inputValue += c;
        showInputScreen();
    }
}

void nextInputStep() {
    displayState.inputStep++;
    
    if (displayState.inputStep < 3 && displayState.stepPrompts[displayState.inputStep].length() > 0) {
        displayState.inputPrompt = displayState.stepPrompts[displayState.inputStep];
        displayState.inputValue = "";
        showInputScreen();
    } else {
        completeMultiStepInput();
    }
}

void completeMultiStepInput() {
    String fullCommand = displayState.pendingAction;
    
    if (displayState.pendingAction == "lorasms_input ") {
        String message = displayState.stepValues[0];
        String includeID = displayState.stepValues[1];
        includeID.toUpperCase();
        
        fullCommand = "lorasms " + message;
        if (includeID.startsWith("N")) {
            fullCommand += " --no-id";
        }
    } else {
        for (int i = 0; i < displayState.inputStep; i++) {
            if (i > 0) fullCommand += " ";
            fullCommand += displayState.stepValues[i];
        }
    }
    
    captureCommandOutput(fullCommand);
    processCommand(&SerialBT, fullCommand);
    
    displayState.multiStepInput = false;
    displayState.inputStep = 0;
    displayState.inputMode = false;
    
    displayCapturedOutput();
    displayState.inMenu = false;
    delay(500);
}

void showMessageOverlay() {
    if (!displayState.messageOverlayActive || displayState.messageQueueCount == 0) {
        return;
    }
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.drawBox(0, 0, 128, 12);
    
    u8g2.setDrawColor(0);
    char header[32];
    snprintf(header, sizeof(header), "MESSAGE (%d)", displayState.messageQueueCount);
    u8g2.drawStr(4, 9, header);
    u8g2.setDrawColor(1);
    
    String currentMsg = displayState.messageQueue[displayState.messageQueueTail];
    
    int y = 24;
    int lineHeight = 10;
    int maxLines = 4;
    int lineCount = 0;
    
    int startPos = 0;
    while (startPos < currentMsg.length() && lineCount < maxLines) {
        int endPos = startPos + 18;
        if (endPos >= currentMsg.length()) {
            endPos = currentMsg.length();
        } else {
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
    
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(4, 60, "Press OK to dismiss");
    
    u8g2.sendBuffer();
}

void dismissCurrentMessage() {
    if (displayState.messageQueueCount > 0) {
        displayState.messageQueueTail = (displayState.messageQueueTail + 1) % 10;
        displayState.messageQueueCount--;
        
        if (displayState.messageQueueCount > 0) {
            showMessageOverlay();
        } else {
            displayState.messageOverlayActive = false;
        }
    }
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
        }
    } else {
        // Queue full - overwrite oldest message
        displayState.messageQueue[displayState.messageQueueHead] = message;
        displayState.messageQueueHead = (displayState.messageQueueHead + 1) % 10;
        displayState.messageQueueTail = (displayState.messageQueueTail + 1) % 10;
    }
}
