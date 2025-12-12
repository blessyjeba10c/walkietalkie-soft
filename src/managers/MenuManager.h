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
void createTrackerMenu();
void createGroundMenu();
void createSettingsMenu();
void createLoRaMenu();
void createEncryptionMenu();
void createDebugMenu();
