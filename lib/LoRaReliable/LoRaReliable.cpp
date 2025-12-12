#include "LoRaReliable.h"

LoRaReliable::LoRaReliable(int ss, int reset, int dio0) {
  _ss = ss;
  _reset = reset;
  _dio0 = dio0;
  _debug = nullptr;
  _lastRSSI = 0;
  _lastSNR = 0.0;
}

bool LoRaReliable::begin(long frequency) {
  LoRa.setPins(_ss, _reset, _dio0);
  
  debugPrint("Initializing LoRa at ");
  debugPrint(String(frequency / 1E6));
  debugPrint(" MHz... ");
  
  if (!LoRa.begin(frequency)) {
    debugPrintln("FAILED!");
    return false;
  }
  
  debugPrintln("SUCCESS!");
  
  // Configure LoRa for reliable communication
  LoRa.setTxPower(20);           // Max power
  LoRa.setSpreadingFactor(7);    // SF7 for faster communication
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.enableCrc();
  
  debugPrintln("LoRa configured: TxPower=20dBm, SF=7, BW=125kHz, CR=4/5, CRC=ON");
  
  return true;
}

void LoRaReliable::setDebugOutput(Stream* debugStream) {
  _debug = debugStream;
}

bool LoRaReliable::send(String message, int maxRetries, unsigned long ackTimeout) {
  debugPrintln("\n=== SENDING MESSAGE ===");
  debugPrint("Payload: ");
  debugPrintln(message);
  debugPrint("Length: ");
  debugPrint(String(message.length()));
  debugPrintln(" bytes");
  debugPrint("Max Retries: ");
  debugPrintln(String(maxRetries));
  debugPrint("ACK Timeout: ");
  debugPrint(String(ackTimeout));
  debugPrintln(" ms");
  
  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    debugPrintln("");
    debugPrint(">>> Attempt ");
    debugPrint(String(attempt));
    debugPrint("/");
    debugPrintln(String(maxRetries));
    
    // Send the packet
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    
    debugPrintln("Packet transmitted");
    debugPrint("Waiting for ACK (");
    debugPrint(String(ackTimeout));
    debugPrintln(" ms)...");
    
    // Wait for acknowledgment
    if (waitForAck(message, ackTimeout)) {
      debugPrintln("\n*** SUCCESS! Message acknowledged ***\n");
      return true;
    }
    
    if (attempt < maxRetries) {
      debugPrintln("ACK not received, retrying...");
      delay(100); // Small delay before retry
    }
  }
  
  debugPrintln("\n*** FAILED! No ACK after all retries ***\n");
  return false;
}

String LoRaReliable::receive() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize == 0) {
    return ""; // No packet
  }
  
  // Read the packet
  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  incoming.trim();
  
  // Get signal quality
  _lastRSSI = LoRa.packetRssi();
  _lastSNR = LoRa.packetSnr();
  
  // Check if this is an ACK message
  if (incoming.startsWith("ACK:")) {
    // This is an ACK, don't process it as a regular message
    return "";
  }
  
  // This is a data message, log it
  debugPrintln("\n=== RECEIVED MESSAGE ===");
  debugPrint("Payload: ");
  debugPrintln(incoming);
  debugPrint("Size: ");
  debugPrint(String(packetSize));
  debugPrintln(" bytes");
  debugPrint("RSSI: ");
  debugPrint(String(_lastRSSI));
  debugPrintln(" dBm");
  debugPrint("SNR: ");
  debugPrint(String(_lastSNR));
  debugPrintln(" dB");
  
  // Send ACK
  sendAck(incoming);
  
  return incoming;
}

int LoRaReliable::getRSSI() {
  return _lastRSSI;
}

float LoRaReliable::getSNR() {
  return _lastSNR;
}

void LoRaReliable::debugPrint(String message) {
  if (_debug) {
    _debug->print(message);
  }
}

void LoRaReliable::debugPrintln(String message) {
  if (_debug) {
    _debug->println(message);
  }
}

bool LoRaReliable::waitForAck(String expectedMessage, unsigned long timeout) {
  unsigned long startTime = millis();
  
  while (millis() - startTime < timeout) {
    int packetSize = LoRa.parsePacket();
    
    if (packetSize) {
      String incoming = "";
      while (LoRa.available()) {
        incoming += (char)LoRa.read();
      }
      incoming.trim();
      
      // Get signal quality
      _lastRSSI = LoRa.packetRssi();
      _lastSNR = LoRa.packetSnr();
      
      debugPrint("Received: ");
      debugPrint(incoming);
      debugPrint(" (RSSI: ");
      debugPrint(String(_lastRSSI));
      debugPrint(" dBm, SNR: ");
      debugPrint(String(_lastSNR));
      debugPrintln(" dB)");
      
      // Check if this is the ACK we're waiting for
      if (incoming.startsWith("ACK:")) {
        String ackContent = incoming.substring(4);
        
        // Simple check: ACK contains part of our message
        if (expectedMessage.indexOf(ackContent) != -1 || 
            ackContent.indexOf(expectedMessage.substring(0, min(20, (int)expectedMessage.length()))) != -1) {
          debugPrintln("ACK matches our message!");
          return true;
        } else {
          debugPrintln("ACK doesn't match (might be for different message)");
        }
      }
    }
    
    delay(10); // Small delay to prevent busy waiting
  }
  
  return false; // Timeout
}

void LoRaReliable::sendAck(String originalMessage) {
  // Create ACK message with truncated original message
  String ackMessage = "ACK:" + originalMessage.substring(0, min(20, (int)originalMessage.length()));
  
  debugPrintln("\n>>> Sending ACK <<<");
  debugPrint("ACK Payload: ");
  debugPrintln(ackMessage);
  
  delay(100); // Small delay before sending ACK
  
  LoRa.beginPacket();
  LoRa.print(ackMessage);
  LoRa.endPacket();
  
  debugPrintln("ACK sent\n");
}
