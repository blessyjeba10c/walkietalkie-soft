#include "DisplayManager.h"

// Global display state and U8g2 object
DisplayState displayState;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

// All other functions are now in the modular files:
// - display/DisplayCore.cpp: initializeDisplay(), updateDisplay(), addMessage(), showMessage(), displayError(), displaySuccess()
// - display/DisplayScreens.cpp: showMainScreen(), showStatusScreen(), showGPSScreen(), showGSMScreen(), showMessageHistory()
// - display/DisplayMenus.cpp: All menu creation and navigation functions
// - display/DisplayInput.cpp: All input handling and message overlay functions
