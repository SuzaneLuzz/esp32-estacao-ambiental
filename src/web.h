#pragma once
#include "sensors.h"
#include "control.h"

void webBegin(const SensorData* data, const ActuatorState* act);
void webLoop();          // não bloqueante
String webIp();
