#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

// Menu navigation functions
void initializeMenus();
void showMenu();
void navigateUp();
void navigateDown();
void selectMenuItem();
void goBack();

// Menu creation functions
void createMainMenu();
void createRadioMenu();
void createRadioConfigMenu();
void createEncryptionMenu();
void createSMSMenu();
void createGPSMenu();
void createGSMMenu();
void createDebugMenu();
void createCommStatusMenu();
void createSettingsMenu();
