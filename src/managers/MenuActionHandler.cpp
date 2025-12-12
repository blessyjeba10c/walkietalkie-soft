#include "MenuActionHandler.h"
#include "MenuManager.h"
#include "InputManager.h"
#include "DisplayManager.h"
#include "CommandProcessor.h"
#include "BluetoothSerial.h"

extern DisplayState displayState;
extern BluetoothSerial SerialBT;

void executeMenuAction(String action) {
    // Handle input actions
    if (action.startsWith("input_")) {
        if (action == "input_call") {
            startInput("Enter Radio ID:", "call ");
        } else if (action == "input_group") {
            startInput("Enter Group ID:", "group ");
        } else if (action == "input_radioid") {
            startInput("Enter Radio ID:", "radioid ");
        } else if (action == "input_channel") {
            startInput("Enter Channel:", "channel ");
        } else if (action == "input_volume") {
            startInput("Enter Volume:", "volume ");
        } else if (action == "input_encryptkey") {
            startInput("Enter Key (32 hex):", "encryptkey ");
        } else if (action == "input_sms") {
            String prompts[] = {"Enter Radio ID:", "Enter Message:"};
            startMultiStepInput("sms ", prompts, 2);
        } else if (action == "input_gsmsms") {
            String prompts[] = {"Enter Phone #:", "Enter Message:"};
            startMultiStepInput("gsmsms ", prompts, 2);
        } else if (action == "input_gsmphone") {
            startInput("Enter Phone #:", "gsmphone ");
        } else if (action == "input_raw") {
            startInput("Enter Raw CMD:", "raw ");
        } else if (action == "input_gps") {
            startInput("Enter Radio ID:", "gps ");
        } else if (action == "input_gpsauto") {
            String prompts[] = {"Enter Radio ID:", "Enter Minutes:", "Enter Seconds:"};
            startMultiStepInput("gpsauto ", prompts, 3);
        } else if (action == "input_soldierid") {
            startInput("Enter Soldier ID:", "soldierid ");
        } else if (action == "input_lorasms") {
            String prompts[] = {"Enter Message:", "Include ID? (Y/N):"};
            startMultiStepInput("lorasms_input ", prompts, 2);
        } else if (action == "input_smartsend") {
            startInput("Enter Message:", "smartsend ");
        } else if (action == "input_groundid") {
            startInput("Enter Ground ID:", "radioid ");
        } else if (action == "input_frequency") {
            startInput("Enter Freq (MHz):", "frequency ");
        }
        return;
    }
    
    // Handle direct commands
    if (action == "exit_menu") {
        displayState.inMenu = false;
        showMainScreen();
    } else if (action == "show_messages") {
        displayState.inMenu = false;
        displayState.currentScreen = "messages";
        showMessageHistory();
    } else if (action == "back") {
        goBack();
    } else if (action == "send_position") {
        startInput("Enter Radio ID:", "gps ");
    } else if (action == "emergency") {
        processCommand(&SerialBT, "emergency");
        displaySuccess("Emergency signal sent");
    } else if (action == "stop") {
        processCommand(&SerialBT, "stop");
        displaySuccess("Call stopped");
    } else if (action == "fallback") {
        processCommand(&SerialBT, "fallback");
        displaySuccess("Fallback mode activated");
    } else if (action == "encrypt on") {
        processCommand(&SerialBT, "encrypt on");
        displaySuccess("Encryption enabled");
    } else if (action == "encrypt off") {
        processCommand(&SerialBT, "encrypt off");
        displaySuccess("Encryption disabled");
    } else if (action == "encrypt status") {
        captureCommandOutput("encrypt status");
        processCommand(&SerialBT, "encrypt status");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "status") {
        captureCommandOutput("status");
        processCommand(&SerialBT, "status");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "info") {
        captureCommandOutput("info");
        processCommand(&SerialBT, "info");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "help") {
        captureCommandOutput("help");
        processCommand(&SerialBT, "help");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "gsmstatus") {
        captureCommandOutput("gsmstatus");
        processCommand(&SerialBT, "gsmstatus");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "smsinfo") {
        captureCommandOutput("smsinfo");
        processCommand(&SerialBT, "smsinfo");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "bt") {
        captureCommandOutput("bt");
        processCommand(&SerialBT, "bt");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "lorastatus") {
        captureCommandOutput("lorastatus");
        processCommand(&SerialBT, "lorastatus");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "ackon") {
        processCommand(&SerialBT, "ackon");
        displaySuccess("LoRa ACK enabled");
    } else if (action == "ackoff") {
        processCommand(&SerialBT, "ackoff");
        displaySuccess("LoRa ACK disabled");
    } else if (action == "fallback") {
        captureCommandOutput("fallback");
        processCommand(&SerialBT, "fallback");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "i2cscan") {
        captureCommandOutput("i2cscan");
        processCommand(&SerialBT, "i2cscan");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "keytest") {
        captureCommandOutput("keytest");
        processCommand(&SerialBT, "keytest");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "keyscan") {
        captureCommandOutput("keyscan");
        processCommand(&SerialBT, "keyscan");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "gps_position") {
        captureCommandOutput("gpsinfo");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "gps_satellites") {
        captureCommandOutput("gpsinfo");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "gps_reset" || action == "gpsstop") {
        captureCommandOutput("gpsstop");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "gsm_signal") {
        captureCommandOutput("gsmstatus");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "system_info") {
        captureCommandOutput("info");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "display_settings") {
        captureCommandOutput("info");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "radio_settings") {
        captureCommandOutput("status");
        displayCapturedOutput();
        displayState.inMenu = false;
        delay(500);
    } else if (action == "debug_mode") {
        displaySuccess("Debug mode toggled");
        displayState.inMenu = false;
        delay(500);
    }
}
