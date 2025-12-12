#include "CommandProcessor.h"
#include "WalkieTalkie.h"
#include "managers/DisplayManager.h"
#include "managers/GPSManager.h"
#include "managers/GSMManager.h"
#include "commands/GSMCommands.h"
#include "commands/GPSCommands.h"
#include "commands/LoRaCommands.h"
#include "commands/EncryptionCommands.h"
#include "commands/DMRCommands.h"
#include "commands/FallbackCommands.h"
#include "commands/DebugCommands.h"
#include "BluetoothSerial.h"
#include "DMR828S.h"

extern DMR828S dmr;
extern WalkieTalkieState wtState;
extern BluetoothSerial SerialBT;
extern DemoMode currentMode;
extern DisplayState displayState;
extern GPSState gpsState;
extern GSMState gsmState;

void processCommand(Stream* stream, String command) {
    // Route to specialized command handlers
    if (command.startsWith("gsm")) {
        handleGSMCommand(stream, command);
    }
    else if (command.startsWith("gps")) {
        handleGPSCommand(stream, command);
    }
    else if (command.startsWith("lora") || command == "ackon" || command == "ackoff") {
        handleLoRaCommand(stream, command);
    }
    else if (command.startsWith("encrypt")) {
        handleEncryptionCommand(stream, command);
    }
    else if (command.startsWith("sms ") || command.startsWith("call ") || 
             command.startsWith("group ") || command == "stop" || 
             command == "emergency" || command.startsWith("channel ") ||
             command.startsWith("volume ") || command.startsWith("radioid ") ||
             command.startsWith("soldierid ") || command.startsWith("raw ")) {
        handleDMRCommand(stream, command);
    }
    else if (command == "fallback" || command.startsWith("smartsend ") ||
             command.startsWith("sendtracker ") || command.startsWith("msgtracker ")) {
        handleFallbackCommand(stream, command);
    }
    else if (command == "i2cscan" || command == "keytest" || command == "keyscan" ||
             command == "smsinfo" || command == "messages" || command == "history" ||
             command == "bt") {
        handleDebugCommand(stream, command);
    }
    else if (command == "status") {
        showStatusTo(stream);
    }
    else if (command == "info") {
        showDeviceInfoTo(stream);
    }
    else if (command == "help") {
        showCommandsTo(stream);
    }
    else {
        stream->println("❓ Unknown command. Type 'help' for commands.");
    }
}

void showCommands() {
    showCommandsTo(&SerialBT);
}

void showCommandsTo(Stream* stream) {
    stream->println("\\n📖 Available Commands:");
    stream->println("=======================");
    stream->println("Communication:");
    stream->println("  sms <hex_id> <message>  - Send SMS");
    stream->println("  call <hex_id>           - Private call");
    stream->println("  group <hex_id>          - Group call");
    stream->println("  stop                    - Stop current call");
    stream->println("  emergency               - Send SOS");
    stream->println("  sendtracker <message>   - Send message to tracker (auto-fallback)");
    stream->println("  smartsend <message>     - Send broadcast message (auto-fallback)");
    stream->println();
    stream->println("Settings:");
    stream->println("  channel <1-16>          - Change channel");
    stream->println("  volume <1-9>            - Set volume");
    stream->println("  radioid <hex>           - Set radio ID");
    stream->println("  encrypt on/off          - Enable/disable encryption");
    stream->println("  encryptkey <16hex>      - Set encryption with custom key");
    stream->println("  encrypt status          - Check encryption status");
    stream->println();
    stream->println("GPS & Location:");
    stream->println("  gps <hex_id>            - Send GPS location once");
    stream->println("  gpsauto <id> <m> <s>    - Auto-send GPS every M:SS");
    stream->println("  gpsstop                 - Stop auto GPS transmission");
    stream->println("  gpsinfo                 - Show GPS status");
    stream->println();
    stream->println("Information:");
    stream->println("  status                  - Show status");
    stream->println("  info                    - Device info");
    stream->println("  smsinfo                 - SMS tracking status");
    stream->println("  fallback                - Manual fallback trigger");
    stream->println("  bt                      - Bluetooth status (BT only)");
    stream->println("  raw <hex>               - Send raw DMR command");
    stream->println("  help                    - Show commands");
    stream->println();
    stream->println("Debug & Testing:");
    stream->println("  i2cscan                 - Scan for I2C devices");
    stream->println("  keytest                 - Test keyboard matrix");
    stream->println("  keyscan                 - Live keyboard scanning test");
    stream->println();
    stream->println("GSM Fallback:");
    stream->println("  gsmstatus               - Check GSM module status");
    stream->println("  gsmcmd <AT_command>     - Send raw AT cmd to GSM");
    stream->println("  gsmphone <number>       - Set fallback phone number");
    stream->println("  gsmsms <number> <msg>   - Send SMS via GSM directly");
    stream->println("  soldierid <id>          - Set soldier identification");
    stream->println();
    stream->println("LoRa Communication:");
    stream->println("  lorastatus              - Check LoRa module status");
    stream->println("  loraid <id>             - Set LoRa network ID (filter)");
    stream->println("  lorasms <message>       - Send message via LoRa");
    stream->println("  loragps <callsign>      - Send GPS location via LoRa");
    stream->println("  ackon                   - Enable LoRa ACK mode");
    stream->println("  ackoff                  - Disable LoRa ACK mode");
    stream->println();
    stream->println("Message History:");
    stream->println("  messages                - View last 6 received messages");
    stream->println("  history                 - Same as messages");
    stream->println();
    stream->println("Examples:");
    stream->println("  sms 123 Hello World");
    stream->println("  call 456");
    stream->println("  channel 5");
    stream->println("  radioid 123456          - Set radio ID to 0x123456");
    stream->println("  gps 123                 - Send location to 0x123");
    stream->println("  gpsauto 123 10          - Auto-send to 0x123 every 10min");
    stream->println("  encrypt on              - Enable encryption");
    stream->println("  encryptkey 0102030405060708 - Custom encryption key");
    stream->println("  raw 68010101A9020110    - Send raw DMR frame");
    stream->println("  gsmphone +1234567890    - Set emergency fallback number");
    stream->println("  soldierid BSF67890      - Set soldier ID to BSF67890");
    stream->println("  loraid NET01            - Set LoRa network ID to NET01");
    stream->println("  lorasms Hello World     - Send message via LoRa");
    stream->println("  loragps Alpha_01        - Send GPS to Alpha_01 via LoRa");
    stream->println("  sendtracker STATUS OK   - Send status to tracker (auto-fallback)");
    stream->println();
}

void showStatus() {
    showStatusTo(&SerialBT);
}

void showStatusTo(Stream* stream) {
    stream->println("\\n📊 Current Status:");
    stream->println("==================");
    stream->print("Mode: ");
    
    switch(currentMode) {
        case MODE_BASIC_TEST: stream->println("Basic Test"); break;
        case MODE_WALKIE_FEATURES: stream->println("Full Walkie-Talkie"); break;
        case MODE_LOW_LEVEL: stream->println("Low-Level Protocol"); break;
        default: stream->println("Unknown"); break;
    }
    
    stream->print("Radio ID: 0x"); stream->println(wtState.myRadioID, HEX);
    stream->print("Channel: "); stream->println(wtState.currentChannel);
    stream->print("Volume: "); stream->println(wtState.volume);
    stream->print("RSSI: "); stream->println(dmr.getRSSI());
    
    DMRModuleStatus status = dmr.getModuleStatus();
    stream->print("Module Status: ");
    switch(status) {
        case STATUS_RECEIVING: stream->println("Receiving"); break;
        case STATUS_TRANSMITTING: stream->println("Transmitting"); break;
        case STATUS_STANDBY: stream->println("Standby"); break;
        default: stream->println("Unknown"); break;
    }
    
    // GPS Status
    stream->println();
    stream->println("📍 GPS Status:");
    stream->print("  Position: ");
    stream->print(gpsState.latitude, 6);
    stream->print(", ");
    stream->println(gpsState.longitude, 6);
    stream->print("  Valid Fix: ");
    stream->println(gpsState.hasValidFix ? "YES" : "NO");
    stream->print("  Auto-transmission: ");
    stream->println(gpsState.continuousMode ? "ENABLED" : "DISABLED");
    
    // GSM Status
    stream->println();
    stream->println("📱 GSM Status:");
    stream->print("  Module: ");
    stream->println(gsmState.initialized ? "READY" : "NOT READY");
    stream->print("  Network: ");
    stream->println(gsmState.networkRegistered ? "REGISTERED" : "NOT REGISTERED");
    stream->print("  Fallback Phone: ");
    stream->println(gsmState.phoneNumber.length() > 0 ? gsmState.phoneNumber : "Not configured");
}

void showDeviceInfo() {
    showDeviceInfoTo(&SerialBT);
}

void showDeviceInfoTo(Stream* stream) {
    stream->println("\\n🔧 Device Information:");
    stream->println("========================");
    stream->println("Hardware: ESP32 DMR Walkie-Talkie");
    stream->println("Firmware: v2.1 - Modular Architecture");
    stream->println("DMR Module: DMR828S");
    stream->println("GPS: Serial0 (9600 baud)");
    stream->println("GSM: Serial1 (9600 baud, GPIO25/26)");
    stream->println("DMR Radio: Serial2 (57600 baud, GPIO16/17)");
    stream->println("Bluetooth: SerialBT (LittleBoyz)");
    stream->println();
    stream->println("Features:");
    stream->println("  ✅ DMR Radio Communication");
    stream->println("  ✅ GPS Location Tracking");
    stream->println("  ✅ GSM Fallback SMS");
    stream->println("  ✅ Bluetooth Wireless Control");
    stream->println("  ✅ Encryption Support");
    stream->println("  ✅ Emergency Alerts");
    stream->println("  ✅ Automatic Fallback System");
    stream->println();
    stream->print("Uptime: ");
    stream->print(millis() / 1000);
    stream->println(" seconds");
}
