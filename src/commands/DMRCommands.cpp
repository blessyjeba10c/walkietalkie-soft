#include "DMRCommands.h"
#include "WalkieTalkie.h"
#include "DMR828S.h"

extern DMR828S dmr;
extern WalkieTalkieState wtState;

void handleDMRCommand(Stream* stream, String command) {
    if (command.startsWith("sms ")) {
        int firstSpace = command.indexOf(' ', 4);
        if (firstSpace > 0) {
            String idStr = command.substring(4, firstSpace);
            String message = command.substring(firstSpace + 1);
            uint32_t targetID = strtoul(idStr.c_str(), NULL, 16);
            if (dmr.sendSMS(targetID, message.c_str())) {
                wtState.currentSendMode = "DMR";
                stream->print("📤 SMS sent to 0x"); stream->print(targetID, HEX);
                stream->print(": "); stream->println(message);
            } else {
                stream->println("❌ SMS send failed");
            }
        }
    }
    else if (command.startsWith("call ")) {
        String idStr = command.substring(5);
        uint32_t targetID = strtoul(idStr.c_str(), NULL, 16);
        if (dmr.startCall(CALL_PRIVATE, targetID)) {
            stream->print("📞 Calling 0x"); stream->println(targetID, HEX);
        }
    }
    else if (command.startsWith("group ")) {
        String idStr = command.substring(6);
        uint32_t targetID = strtoul(idStr.c_str(), NULL, 16);
        if (dmr.startCall(CALL_GROUP, targetID)) {
            stream->print("📞 Group call to 0x"); stream->println(targetID, HEX);
        }
    }
    else if (command == "stop") {
        dmr.stopCall();
        stream->println("📞 Call stopped");
    }
    else if (command == "emergency") {
        if (dmr.sendEmergencyAlarm(0)) {
            stream->println("🚨 Emergency alert sent");
        }
    }
    else if (command.startsWith("channel ")) {
        int ch = command.substring(8).toInt();
        if (ch >= 1 && ch <= 16) {
            wtState.currentChannel = ch;
            if (dmr.setChannel(ch)) {
                stream->print("📻 Channel: "); stream->println(ch);
            }
        }
    }
    else if (command.startsWith("volume ")) {
        int vol = command.substring(7).toInt();
        if (vol >= 1 && vol <= 9) {
            wtState.volume = vol;
            if (dmr.setVolume(vol)) {
                stream->print("🔊 Volume: "); stream->println(vol);
            }
        }
    }
    else if (command.startsWith("radioid ")) {
        String idStr = command.substring(8);
        uint32_t radioID = strtoul(idStr.c_str(), NULL, 16);
        
        if (radioID > 0 && radioID <= 0xFFFFFF) {
            wtState.myRadioID = radioID;
            if (dmr.setRadioID(radioID)) {
                stream->print("🆔 Radio ID: 0x"); stream->println(radioID, HEX);
            } else {
                stream->println("❌ Failed to set Radio ID");
            }
        } else {
            stream->println("❌ Invalid Radio ID. Use hex format (1-FFFFFF)");
        }
    }
    else if (command.startsWith("soldierid ")) {
        String soldierID = command.substring(10);
        soldierID.trim();
        if (soldierID.length() > 0) {
            wtState.soldierID = soldierID;
            stream->println("✅ Soldier ID set: " + soldierID);
        } else {
            stream->println("❌ Invalid soldier ID");
        }
    }
    else if (command.startsWith("raw ")) {
        String hexStr = command.substring(4);
        hexStr.trim();
        hexStr.toUpperCase();
        
        if (hexStr.length() % 2 == 0 && hexStr.length() >= 2) {
            int dataLen = hexStr.length() / 2;
            uint8_t* rawData = new uint8_t[dataLen];
            bool validHex = true;
            
            for (int i = 0; i < dataLen; i++) {
                String byteStr = hexStr.substring(i*2, i*2+2);
                char* endptr;
                unsigned long val = strtoul(byteStr.c_str(), &endptr, 16);
                if (*endptr != '\0' || val > 255) {
                    validHex = false;
                    break;
                }
                rawData[i] = (uint8_t)val;
            }
            
            if (validHex) {
                DMR828S_Utils& lowLevel = dmr.getLowLevel();
                
                // Send raw frame - first byte is command, rest is data
                if (dataLen >= 1) {
                    uint8_t cmd = rawData[0];
                    uint8_t* frameData = (dataLen > 1) ? &rawData[1] : nullptr;
                    int frameDataLen = (dataLen > 1) ? dataLen - 1 : 0;
                    
                    if (lowLevel.sendFrame(cmd, 0x01, 0x01, frameData, frameDataLen)) {
                        stream->print("📡 Raw command sent: ");
                        for (int i = 0; i < dataLen; i++) {
                            if (rawData[i] < 0x10) stream->print("0");
                            stream->print(rawData[i], HEX);
                            stream->print(" ");
                        }
                        stream->println();
                    } else {
                        stream->println("❌ Failed to send raw command");
                    }
                } else {
                    stream->println("❌ Need at least command byte");
                }
            } else {
                stream->println("❌ Invalid hex format. Use: raw 68010000XX...");
            }
            
            delete[] rawData;
        } else {
            stream->println("❌ Invalid hex length. Must be even number of hex digits.");
        }
    }
}
