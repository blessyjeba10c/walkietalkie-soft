#ifndef LORA_RELIABLE_H
#define LORA_RELIABLE_H

#include <Arduino.h>
#include <LoRa.h>

class LoRaReliable {
public:
  // Constructor
  LoRaReliable(int ss, int reset, int dio0);
  
  // Initialize LoRa with frequency
  bool begin(long frequency);
  
  // Set debug output stream (e.g., &SerialBT)
  void setDebugOutput(Stream* debugStream);
  
  // Send message with automatic retry and ACK handling
  // Returns true if message was acknowledged, false if failed after retries
  bool send(String message, int maxRetries = 3, unsigned long ackTimeout = 2000);
  
  // Check for incoming messages (call this in loop)
  // Returns received message, or empty string if none
  String receive();
  
  // Get last RSSI
  int getRSSI();
  
  // Get last SNR
  float getSNR();

private:
  int _ss;
  int _reset;
  int _dio0;
  Stream* _debug;
  int _lastRSSI;
  float _lastSNR;
  
  // Internal functions
  void debugPrint(String message);
  void debugPrintln(String message);
  bool waitForAck(String expectedMessage, unsigned long timeout);
  void sendAck(String originalMessage);
};

#endif // LORA_RELIABLE_H
