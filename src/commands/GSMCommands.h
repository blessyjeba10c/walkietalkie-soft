#pragma once
#include <Arduino.h>
#include "../managers/GSMManager.h"

void handleGSMCommand(Stream* stream, String command);
void refreshGSMState();
String sendGSMATCommand(const String& cmd, unsigned long timeout = 1000);
