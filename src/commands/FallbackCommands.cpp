#include "FallbackCommands.h"
#include "WalkieTalkie.h"
#include "managers/GSMManager.h"
#include "managers/LoRaManager.h"
#include "DMR828S.h"

extern DMR828S dmr;
extern WalkieTalkieState wtState;
extern GSMState gsmState;

void handleFallbackCommand(Stream* stream, String command) {
    if (command == "fallback") {
        stream->println("🔄 Testing communication fallback systems:");
        
        // Test DMR status
        DMRModuleStatus dmrStatus = dmr.getModuleStatus();
        bool dmrOk = (dmrStatus == STATUS_STANDBY || dmrStatus == STATUS_RECEIVING);
        stream->print("📻 DMR Radio: ");
        stream->println(dmrOk ? "✅ AVAILABLE" : "❌ UNAVAILABLE");
        
        // Test LoRa status  
        bool loraOk = isLoRaAvailable();
        stream->print("📡 LoRa: ");
        stream->println(loraOk ? "✅ AVAILABLE" : "❌ UNAVAILABLE");
        
        // Test GSM status
        bool gsmOk = (gsmState.initialized && gsmState.networkRegistered);
        stream->print("📱 GSM: ");
        stream->println(gsmOk ? "✅ AVAILABLE" : "❌ UNAVAILABLE");
        
        // Recommend communication method
        stream->println();
        if (dmrOk) {
            stream->println("📻 Recommended: Use DMR radio (primary)");
        } else if (loraOk) {
            stream->println("📡 Recommended: Use LoRa (secondary fallback)");
        } else if (gsmOk) {
            stream->println("📱 Recommended: Use GSM SMS (final fallback)");
        } else {
            stream->println("🚨 WARNING: All communication methods unavailable!");
        }
    }
    else if (command.startsWith("smartsend ")) {
        String message = command.substring(10);
        message.trim();
        
        if (message.length() > 0) {
            bool sent = false;
            
            // Try DMR first (if we have a target ID)
            if (!sent) {
                stream->println("📻 Trying DMR radio...");
                // Would need target ID - skip for now
                stream->println("⏩ Skipping DMR (requires target ID)");
            }
            
            // Try LoRa second
            if (!sent && isLoRaAvailable()) {
                stream->println("📡 Trying LoRa fallback...");
                if (sendLoRaMessage("BROADCAST: " + message)) {
                    stream->println("✅ Message sent via LoRa");
                    sent = true;
                }
            }
            
            // Try GSM last
            if (!sent && gsmState.initialized && gsmState.networkRegistered) {
                stream->println("📱 Trying GSM fallback...");
                if (gsmState.phoneNumber.length() > 0) {
                    sendGSMFallbackSMS(gsmState.phoneNumber, message);
                    stream->println("✅ Message sent via GSM SMS");
                    sent = true;
                } else {
                    stream->println("❌ GSM phone number not configured");
                }
            }
            
            if (!sent) {
                stream->println("❌ All communication methods failed");
            }
        } else {
            stream->println("❌ Format: smartsend <message>");
        }
    }
    else if (command.startsWith("sendtracker ") || command.startsWith("msgtracker ")) {
        // Extract message (works with both commands)
        int spacePos = command.indexOf(' ');
        String message = command.substring(spacePos + 1);
        message.trim();
        
        if (message.length() > 0) {
            stream->println("📡 Sending message to tracker via fallback system...");
            bool sent = false;
            
            // Try DMR first if target ID is set
            if (!sent && wtState.myRadioID != 0) {
                stream->println("📻 Attempting DMR transmission...");
                if (dmr.sendSMS(wtState.myRadioID, message.c_str())) {
                    stream->println("✅ Message sent via DMR");
                    sent = true;
                } else {
                    stream->println("⚠️ DMR transmission failed, trying fallback...");
                }
            }
            
            // Try LoRa second
            if (!sent && isLoRaAvailable()) {
                stream->println("📡 Attempting LoRa transmission...");
                if (sendLoRaMessage(message)) {
                    stream->println("✅ Message sent via LoRa");
                    sent = true;
                } else {
                    stream->println("⚠️ LoRa transmission failed, trying GSM...");
                }
            }
            
            // Try GSM last
            if (!sent && gsmState.initialized && gsmState.networkRegistered) {
                stream->println("📱 Attempting GSM transmission...");
                if (gsmState.phoneNumber.length() > 0) {
                    sendGSMFallbackSMS(gsmState.phoneNumber, message);
                    stream->println("✅ Message sent via GSM SMS");
                    sent = true;
                } else {
                    stream->println("❌ GSM phone number not configured");
                }
            }
            
            if (!sent) {
                stream->println("❌ All communication methods failed");
                stream->println("💡 Tip: Configure fallback phone with 'gsmphone <number>'");
            }
        } else {
            stream->println("❌ Format: sendtracker <message>");
        }
    }
}
