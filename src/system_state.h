#pragma once
#include "schedule.h"

enum MotorCommand {
  MotorsStop,
  MotorsForward,
  MotorsBackward,
  MotorsTurnLeft,
  MotorsTurnRight
};

struct SystemInputs {
  bool modeButtonPressed;
  bool pirMotionDetected;
  bool btnForward;
  bool btnBackward;
  bool btnLeft;
  bool btnRight;
  int hour;
  int minute;
};

struct SystemState {
  bool autoMode = true;          // start in AUTO to mirror firmware
  bool relayOn = false;
  MotorCommand motors = MotorsStop;
};

inline bool withinManualButtonRange(const SystemInputs& in) {
  return in.btnForward || in.btnBackward || in.btnLeft || in.btnRight;
}

inline MotorCommand manualCommand(const SystemInputs& in) {
  if (in.btnForward) return MotorsForward;
  if (in.btnBackward) return MotorsBackward;
  if (in.btnLeft) return MotorsTurnLeft;
  if (in.btnRight) return MotorsTurnRight;
  return MotorsStop;
}

inline void updateSystem(SystemState& st, const SystemInputs& in) {
  if (in.modeButtonPressed) {
    st.autoMode = !st.autoMode;
  }

  st.relayOn = !in.pirMotionDetected;

  if (st.autoMode) {
    if (inAutoWindow(in.hour, in.minute)) {
      st.motors = MotorsForward;
    } else {
      st.motors = MotorsStop;
    }
  } else {
    st.motors = withinManualButtonRange(in)
                  ? manualCommand(in)
                  : MotorsStop;
  }
}

