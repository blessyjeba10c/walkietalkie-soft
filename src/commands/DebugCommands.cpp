#include "DebugCommands.h"
#include "managers/KeyboardManager.h"
#include "managers/DisplayManager.h"

extern DisplayState displayState;

void handleDebugCommand(Stream* stream, String command) {
    if (command == "i2cscan") {
        scanI2CDevices();
    }
    else if (command == "keytest") {
        testKeyboard();
    }
    else if (command == "keyscan") {
        stream->println("Keyboard scanning mode enabled. Watch for key presses...");
        for (int i = 0; i < 50; i++) { // Scan for 1 second (50 * 20ms)
            scanKeyboard();
            delay(20);
        }
        stream->println("Keyboard scanning test complete.");
    }
    else if (command == "smsinfo") {
        stream->println("\\n📊 SMS Status Info:");
        stream->println("Use this to check if waiting for SMS response");
    }
    else if (command == "messages" || command == "history") {
        stream->println("\\n📨 Message History (Last 6):");
        for (int i = 0; i < 6; i++) {
            int msgIndex = (displayState.messageIndex - 1 - i + 6) % 6;
            if (displayState.messages[msgIndex].length() > 0) {
                stream->print(String(i + 1) + ". ");
                stream->println(displayState.messages[msgIndex]);
            }
        }
    }
    else if (command == "bt") {
        // Bluetooth specific command
        stream->println("📶 Bluetooth Status: Connected");
        stream->println("Device Name: DMR828S-Walkie");
    }
}
