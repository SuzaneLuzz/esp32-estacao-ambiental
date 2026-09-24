#pragma once
#include "sensors.h"
#include "control.h"

constexpr uint8_t DISPLAY_PAGES = 3;

bool displayBegin();
void displayUpdate(const SensorData& d, const ActuatorState& a, uint8_t page, const String& ip);
