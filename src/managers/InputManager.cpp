#include "InputManager.h"
#include "MenuManager.h"
#include "DisplayManager.h"
#include "CommandProcessor.h"
#include "BluetoothSerial.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern DisplayState displayState;
extern BluetoothSerial SerialBT;

// =============== OUTPUT CAPTURE SYSTEM ===============

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

// =============== INPUT SYSTEM ===============

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

// =============== MULTI-STEP INPUT SYSTEM ===============

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
