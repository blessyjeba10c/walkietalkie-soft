#include "MenuManager.h"
#include "MenuActionHandler.h"
#include "DisplayManager.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern DisplayState displayState;

void initializeMenus() {
    createMainMenu();
}

void createMainMenu() {
    displayState.currentMenu.title = "Main Menu";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 3;
    
    displayState.currentMenu.items[0] = {"TRACKER", "tracker_menu", true};
    displayState.currentMenu.items[1] = {"GROUND", "ground_menu", true};
    displayState.currentMenu.items[2] = {"SETTINGS", "settings_menu", true};
}

void createTrackerMenu() {
    displayState.currentMenu.title = "TRACKER Setup";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 6;
    
    displayState.currentMenu.items[0] = {"Set Radio ID", "input_radioid", false};
    displayState.currentMenu.items[1] = {"Set Ground ID", "input_groundid", false};
    displayState.currentMenu.items[2] = {"Set GSM Phone", "input_gsmphone", false};
    displayState.currentMenu.items[3] = {"Auto GPS Time", "input_gpsauto", false};
    displayState.currentMenu.items[4] = {"Send Position", "send_position", false};
    displayState.currentMenu.items[5] = {"Back", "back", false};
}

void createGroundMenu() {
    displayState.currentMenu.title = "GROUND Setup";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 4;
    
    displayState.currentMenu.items[0] = {"Set Radio ID", "input_radioid", false};
    displayState.currentMenu.items[1] = {"Set Channel", "input_channel", false};
    displayState.currentMenu.items[2] = {"Radio Status", "status", false};
    displayState.currentMenu.items[3] = {"Back", "back", false};
}

void createRadioMenu() {
    displayState.currentMenu.title = "Radio Control";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 8;
    
    displayState.currentMenu.items[0] = {"Call Radio ID", "input_call", false};
    displayState.currentMenu.items[1] = {"Group Call", "input_group", false};
    displayState.currentMenu.items[2] = {"Emergency Call", "emergency", false};
    displayState.currentMenu.items[3] = {"Stop Call", "stop", false};
    displayState.currentMenu.items[4] = {"Send Position", "send_position", false};
    displayState.currentMenu.items[5] = {"Radio Status", "status", false};
    displayState.currentMenu.items[6] = {"Fallback Mode", "fallback", false};
    displayState.currentMenu.items[7] = {"Back", "back", false};
}

void createGPSMenu() {
    displayState.currentMenu.title = "GPS Control";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 6;
    
    displayState.currentMenu.items[0] = {"GPS Status", "gps_position", false};
    displayState.currentMenu.items[1] = {"Send GPS Once", "input_gps", false};
    displayState.currentMenu.items[2] = {"Auto GPS Send", "input_gpsauto", false};
    displayState.currentMenu.items[3] = {"Stop Auto GPS", "gpsstop", false};
    displayState.currentMenu.items[4] = {"Detailed Info", "gps_satellites", false};
    displayState.currentMenu.items[5] = {"Back", "back", false};
}

void createGSMMenu() {
    displayState.currentMenu.title = "GSM Control";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 4;
    
    displayState.currentMenu.items[0] = {"Network Status", "gsmstatus", false};
    displayState.currentMenu.items[1] = {"Signal Strength", "gsmstatus", false};
    displayState.currentMenu.items[2] = {"Send SMS Test", "input_gsmsms", false};
    displayState.currentMenu.items[3] = {"Back", "back", false};
}

void createRadioConfigMenu() {
    displayState.currentMenu.title = "Radio Config";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 6;
    
    displayState.currentMenu.items[0] = {"Set Radio ID", "input_radioid", false};
    displayState.currentMenu.items[1] = {"Set Channel", "input_channel", false};
    displayState.currentMenu.items[2] = {"Set Volume", "input_volume", false};
    displayState.currentMenu.items[3] = {"Radio Info", "info", false};
    displayState.currentMenu.items[4] = {"Send Raw CMD", "input_raw", false};
    displayState.currentMenu.items[5] = {"Back", "back", false};
}

void createEncryptionMenu() {
    displayState.currentMenu.title = "Encryption";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 5;
    
    displayState.currentMenu.items[0] = {"Enable Encrypt", "encrypt on", false};
    displayState.currentMenu.items[1] = {"Disable Encrypt", "encrypt off", false};
    displayState.currentMenu.items[2] = {"Set Encrypt Key", "input_encryptkey", false};
    displayState.currentMenu.items[3] = {"Encrypt Status", "encrypt status", false};
    displayState.currentMenu.items[4] = {"Back", "back", false};
}

void createSMSMenu() {
    displayState.currentMenu.title = "Send SMS";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 4;
    
    displayState.currentMenu.items[0] = {"Send DMR SMS", "input_sms", false};
    displayState.currentMenu.items[1] = {"Send LoRa SMS", "input_lorasms", false};
    displayState.currentMenu.items[2] = {"Send GSM SMS", "input_gsmsms", false};
    displayState.currentMenu.items[3] = {"Back", "back", false};
}

void createDebugMenu() {
    displayState.currentMenu.title = "Debug Tools";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 6;
    
    displayState.currentMenu.items[0] = {"I2C Scan", "i2cscan", false};
    displayState.currentMenu.items[1] = {"Key Test", "keytest", false};
    displayState.currentMenu.items[2] = {"Key Scan", "keyscan", false};
    displayState.currentMenu.items[3] = {"Bluetooth Info", "bt", false};
    displayState.currentMenu.items[4] = {"Help", "help", false};
    displayState.currentMenu.items[5] = {"Back", "back", false};
}

void createCommStatusMenu() {
    displayState.currentMenu.title = "Comm Status";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 5;
    
    displayState.currentMenu.items[0] = {"DMR Status", "status", false};
    displayState.currentMenu.items[1] = {"LoRa Status", "lorastatus", false};
    displayState.currentMenu.items[2] = {"GSM Status", "gsmstatus", false};
    displayState.currentMenu.items[3] = {"Fallback Test", "fallback", false};
    displayState.currentMenu.items[4] = {"Back", "back", false};
}

void createSettingsMenu() {
    displayState.currentMenu.title = "SETTINGS";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 9;
    
    displayState.currentMenu.items[0] = {"Send SMS", "sms_menu", true};
    displayState.currentMenu.items[1] = {"Set Channel", "input_channel", false};
    displayState.currentMenu.items[2] = {"Set Volume", "input_volume", false};
    displayState.currentMenu.items[3] = {"Set Frequency", "input_frequency", false};
    displayState.currentMenu.items[4] = {"GSM Phone", "input_gsmphone", false};
    displayState.currentMenu.items[5] = {"LoRa Config", "lora_menu", true};
    displayState.currentMenu.items[6] = {"Encryption", "encryption_menu", true};
    displayState.currentMenu.items[7] = {"Debug Tools", "debug_menu", true};
    displayState.currentMenu.items[8] = {"Back", "back", false};
}

void createLoRaMenu() {
    displayState.currentMenu.title = "LoRa Config";
    displayState.currentMenu.selectedItem = 0;
    displayState.currentMenu.itemCount = 5;
    
    displayState.currentMenu.items[0] = {"Set Network ID", "input_loraid", false};
    displayState.currentMenu.items[1] = {"LoRa Status", "lorastatus", false};
    displayState.currentMenu.items[2] = {"Enable ACK", "ackon", false};
    displayState.currentMenu.items[3] = {"Disable ACK", "ackoff", false};
    displayState.currentMenu.items[4] = {"Back", "back", false};
}

void showMenu() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Title
    u8g2.drawStr(0, 10, displayState.currentMenu.title.c_str());
    u8g2.drawHLine(0, 12, 128);
    
    // Menu items (show up to 5 items)
    int startItem = displayState.currentMenu.selectedItem > 2 ? displayState.currentMenu.selectedItem - 2 : 0;
    int endItem = min(startItem + 5, displayState.currentMenu.itemCount);
    
    for (int i = startItem; i < endItem; i++) {
        int y = 25 + (i - startItem) * 10;
        
        // Selection indicator
        if (i == displayState.currentMenu.selectedItem) {
            u8g2.drawBox(0, y - 8, 128, 9);
            u8g2.setColorIndex(0); // Invert color for selected item
        }
        
        // Menu item text
        String itemText = displayState.currentMenu.items[i].title;
        if (displayState.currentMenu.items[i].isSubmenu) {
            itemText += " >";
        }
        u8g2.drawStr(2, y, itemText.c_str());
        
        if (i == displayState.currentMenu.selectedItem) {
            u8g2.setColorIndex(1); // Reset color
        }
    }
    
    // Navigation help
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr(0, 62, "2/8:Up/Down 5:Select #:Back");
    
    u8g2.sendBuffer();
}

void navigateUp() {
    if (displayState.currentMenu.selectedItem > 0) {
        displayState.currentMenu.selectedItem--;
        showMenu();
    }
}

void navigateDown() {
    if (displayState.currentMenu.selectedItem < displayState.currentMenu.itemCount - 1) {
        displayState.currentMenu.selectedItem++;
        showMenu();
    }
}

void selectMenuItem() {
    MenuItem selectedItem = displayState.currentMenu.items[displayState.currentMenu.selectedItem];
    
    if (selectedItem.isSubmenu) {
        // Push current menu to stack
        if (displayState.menuStackDepth < 4) {
            displayState.menuStack[displayState.menuStackDepth] = displayState.currentMenu;
            displayState.menuStackDepth++;
        }
        
        // Load submenu
        if (selectedItem.action == "tracker_menu") {
            createTrackerMenu();
        } else if (selectedItem.action == "ground_menu") {
            createGroundMenu();
        } else if (selectedItem.action == "settings_menu") {
            createSettingsMenu();
        } else if (selectedItem.action == "sms_menu") {
            createSMSMenu();
        } else if (selectedItem.action == "lora_menu") {
            createLoRaMenu();
        } else if (selectedItem.action == "encryption_menu") {
            createEncryptionMenu();
        } else if (selectedItem.action == "debug_menu") {
            createDebugMenu();
        }
        
        showMenu();
    } else {
        executeMenuAction(selectedItem.action);
    }
}

void goBack() {
    if (displayState.menuStackDepth > 0) {
        // Pop menu from stack
        displayState.menuStackDepth--;
        displayState.currentMenu = displayState.menuStack[displayState.menuStackDepth];
        showMenu();
    } else {
        // Exit menu system
        displayState.inMenu = false;
        showMainScreen();
    }
}
