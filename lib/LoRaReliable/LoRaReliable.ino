/*
 * LoRaReliable Test Sketch
 * 
 * Tests the LoRaReliable library with automatic ACK and retry functionality.
 * 
 * SETUP:
 * 1. Set MODE below to either SENDER or RECEIVER
 * 2. Upload to ESP32 boards
 * 3. Connect via Bluetooth Serial to see debug output
 * 
 * SENDER MODE: Sends messages every 10 seconds with automatic retry
 * RECEIVER MODE: Receives messages and automatically sends ACK
 * 
 * WIRING (ESP32 + LoRa SX1276/SX1278):
 *   LoRa SCK  -> GPIO 5
 *   LoRa MISO -> GPIO 19
 *   LoRa MOSI -> GPIO 27
 *   LoRa CS   -> GPIO 18
 *   LoRa RST  -> GPIO 14
 *   LoRa DIO0 -> GPIO 26
 */

#include <SPI.h>
#include "LoRaReliable.h"
#include "BluetoothSerial.h"

// ============ CONFIGURATION ============
// Change this to switch between modes
//#define MODE_SENDER      // Use MODE_SENDER or MODE_RECEIVER
#define MODE_RECEIVER

// LoRa Pin Configuration
#define LORA_SCK     18
#define LORA_MISO    19
#define LORA_MOSI    23
#define LORA_SS      5
#define LORA_RST     14
#define LORA_DIO0    27
#define LORA_FREQ    433E6  // 433 MHz (change to 915E6 for 915 MHz)

// Timing Configuration
#define SEND_INTERVAL 10000  // Send message every 10 seconds

// ============ OBJECTS ============
BluetoothSerial SerialBT;
LoRaReliable lora(LORA_SS, LORA_RST, LORA_DIO0);

// ============ GLOBAL VARIABLES ============
unsigned long lastSendTime = 0;
int messageCounter = 0;

// ============ SETUP ============
void setup() {
  // Initialize Serial (for USB debugging)
  Serial.begin(115200);
  delay(1000);
  
  // Initialize Bluetooth Serial
  #ifdef MODE_SENDER
  SerialBT.begin("LoRa_Sender");
  Serial.println("Bluetooth: LoRa_Sender");
  #else
  SerialBT.begin("LoRa_Receiver");
  Serial.println("Bluetooth: LoRa_Receiver");
  #endif
  
  delay(2000); // Give BT time to start
  
  // Print header
  SerialBT.println("\n\n========================================");
  SerialBT.println("     LoRaReliable Test Sketch");
  SerialBT.println("========================================");
  
  #ifdef MODE_SENDER
  SerialBT.println("Mode: SENDER");
  #else
  SerialBT.println("Mode: RECEIVER");
  #endif
  
  SerialBT.print("Frequency: ");
  SerialBT.print(LORA_FREQ / 1E6);
  SerialBT.println(" MHz");
  SerialBT.println("========================================\n");
  
  // Initialize SPI
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  
  // Set debug output to Bluetooth Serial
  lora.setDebugOutput(&SerialBT);
  
  // Initialize LoRa
  if (!lora.begin(LORA_FREQ)) {
    SerialBT.println("ERROR: LoRa initialization failed!");
    SerialBT.println("Check wiring and restart.");
    while (1) {
      delay(1000);
    }
  }
  
  SerialBT.println("\n========================================");
  SerialBT.println("         System Ready!");
  SerialBT.println("========================================\n");
  
  #ifdef MODE_SENDER
  SerialBT.println("Will send messages every 10 seconds...\n");
  #else
  SerialBT.println("Listening for messages...\n");
  #endif
}

// ============ LOOP ============
void loop() {
  #ifdef MODE_SENDER
  // SENDER MODE: Send messages periodically
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = millis();
    
    messageCounter++;
    String message = "Hello LoRa #" + String(messageCounter) + " @ " + String(millis());
    
    SerialBT.println("┌────────────────────────────────────┐");
    SerialBT.print("│ Message #");
    SerialBT.print(messageCounter);
    for (int i = String(messageCounter).length(); i < 25; i++) SerialBT.print(" ");
    SerialBT.println("│");
    SerialBT.println("└────────────────────────────────────┘");
    
    // Send with automatic retry (3 attempts, 2 second timeout)
    bool success = lora.send(message, 3, 2000);
    
    if (success) {
      SerialBT.println("✓ Message delivered successfully!");
    } else {
      SerialBT.println("✗ Message delivery FAILED after retries!");
    }
    
    SerialBT.println("\n--- Waiting for next send interval ---\n");
  }
  
  #else
  // RECEIVER MODE: Listen for messages
  String receivedMessage = lora.receive();
  
  if (receivedMessage != "") {
    SerialBT.println("┌────────────────────────────────────┐");
    SerialBT.println("│ NEW MESSAGE RECEIVED               │");
    SerialBT.println("└────────────────────────────────────┘");
    SerialBT.print("Content: ");
    SerialBT.println(receivedMessage);
    SerialBT.print("RSSI: ");
    SerialBT.print(lora.getRSSI());
    SerialBT.println(" dBm");
    SerialBT.print("SNR: ");
    SerialBT.print(lora.getSNR());
    SerialBT.println(" dB");
    SerialBT.println("\n--- Waiting for next message ---\n");
  }
  #endif
  
  delay(10);
}
