#pragma once
#include "sensors.h"

enum class AlarmState : uint8_t { OK, WARNING, CRITICAL };

struct ActuatorState {
  bool fan   = false;
  bool led   = false;
  bool muted = false;
  AlarmState alarm = AlarmState::OK;
};

void controlBegin();
void controlUpdate(const SensorData& d, ActuatorState& a);
void controlToggleMute();
const char* alarmToStr(AlarmState s);
