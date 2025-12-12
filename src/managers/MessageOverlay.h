#pragma once

#include <Arduino.h>

// Message overlay functions
void addMessageToQueue(String message);
void showMessageOverlay();
void dismissCurrentMessage();
bool hasMessagesInQueue();
