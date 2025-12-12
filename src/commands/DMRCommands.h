#pragma once

#include <Arduino.h>
#include <Stream.h>

// DMR Radio command handlers
void handleDMRCommand(Stream* stream, String command);
