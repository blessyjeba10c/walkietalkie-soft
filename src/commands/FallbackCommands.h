#pragma once

#include <Arduino.h>
#include <Stream.h>

// Fallback system command handlers
void handleFallbackCommand(Stream* stream, String command);
