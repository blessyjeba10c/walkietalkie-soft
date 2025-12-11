#include "EncryptionCommands.h"
#include "../../include/WalkieTalkie.h"
#include "DMR828S.h"

extern DMR828S dmr;

void handleEncryptionCommand(Stream* stream, String command) {
    if (command == "encrypt on") {
        if (dmr.setEncryption(true)) {
            stream->println("🔒 Encryption: ON (using default key)");
        } else {
            stream->println("❌ Failed to enable encryption");
        }
    }
    else if (command == "encrypt off") {
        if (dmr.setEncryption(false)) {
            stream->println("🔓 Encryption: OFF");
        } else {
            stream->println("❌ Failed to disable encryption");
        }
    }
    else if (command.startsWith("encryptkey ")) {
        String keyStr = command.substring(11);
        keyStr.trim();
        
        if (keyStr.length() == 16) {
            uint8_t key[8];
            bool validHex = true;
            
            for (int i = 0; i < 8; i++) {
                String byteStr = keyStr.substring(i*2, i*2+2);
                char* endptr;
                unsigned long val = strtoul(byteStr.c_str(), &endptr, 16);
                if (*endptr != '\0' || val > 255) {
                    validHex = false;
                    break;
                }
                key[i] = (uint8_t)val;
            }
            
            if (validHex) {
                if (dmr.setEncryption(true, key)) {
                    stream->print("🔒 Encryption: ON with custom key: ");
                    for (int i = 0; i < 8; i++) {
                        if (key[i] < 0x10) stream->print("0");
                        stream->print(key[i], HEX);
                    }
                    stream->println();
                } else {
                    stream->println("❌ Failed to set encryption key");
                }
            } else {
                stream->println("❌ Invalid hex key format. Use 16 hex digits (8 bytes)");
            }
        } else {
            stream->println("❌ Key must be 16 hex digits (8 bytes). Example: 0123456789ABCDEF");
        }
    }
    else if (command == "encrypt status") {
        // Get current encryption status
        stream->println("🔐 Encryption Status: [Feature available - check DMR module]");
    }
}