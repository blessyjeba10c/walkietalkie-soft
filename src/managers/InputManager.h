#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

// Input handling functions
void startInput(String prompt, String action);
void handleInput(char c);
void cancelInput();
void confirmInput();
void showInputScreen();

// Multi-step input functions
void startMultiStepInput(String action, String prompts[], int stepCount);
void handleMultiStepInput(char c);
void nextInputStep();
void completeMultiStepInput();

// Output capture functions
void captureCommandOutput(String command);
void displayCapturedOutput();
