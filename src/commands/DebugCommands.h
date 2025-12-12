#pragma once

#include <Arduino.h>
#include <Stream.h>

// Debug and diagnostic command handlers
void handleDebugCommand(Stream* stream, String command);
