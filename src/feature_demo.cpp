#include "WalkieTalkie.h"
#include "managers/DisplayManager.h"
#include "managers/KeyboardManager.h"
#include "managers/GSMManager.h"

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    
    // Initialize Display
    initializeDisplay();
    Serial.println("Display initialized");

    // Initialize Keyboard
    initializeKeyboard();
    Serial.println("Keyboard initialized");

    // Initialize GSM
    Serial1.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
    initializeGSM();
    Serial.println("GSM initialized");
}

void loop() {
    // Example: Display a message
    showMessageOnDisplay("Hello from ESP32!");
    delay(2000);

    // Example: Check for keyboard input (pseudo)
    // You can add your own logic here
    // if (isKeyPressed()) {
    //     Serial.println("Key pressed!");
    // }

    // Example: Check GSM status
    if (gsmState.initialized) {
        Serial.println("GSM is ready");
    } else {
        Serial.println("GSM not ready");
    }
    delay(3000);
}
