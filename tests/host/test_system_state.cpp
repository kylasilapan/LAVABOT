#include <AUnit.h>
#include "../../src/system_state.h"

using namespace aunit;

test(ModeToggleChangesBehavior) {
  SystemState st;
  SystemInputs in = {};

  in.modeButtonPressed = true;
  updateSystem(st, in);                 // toggle -> manual
  assertFalse(st.autoMode);

  in.modeButtonPressed = false;
  in.btnForward = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  in.btnForward = false;
  in.modeButtonPressed = true;
  updateSystem(st, in);                 // toggle back -> auto
  assertTrue(st.autoMode);

  in.modeButtonPressed = false;
  in.hour = 10;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
}

test(PIRSafetyCutoff) {
  SystemState st;
  SystemInputs in = {};

  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertTrue(st.relayOn);

  in.pirMotionDetected = true;
  updateSystem(st, in);
  assertFalse(st.relayOn);
}

test(AutoScheduleWindow) {
  SystemState st;
  SystemInputs in = {};

  st.autoMode = true;
  in.hour = 9;
  in.minute = 30;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  in.hour = 11;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
}

test(ManualPriority) {
  SystemState st;
  SystemInputs in = {};

  st.autoMode = false;
  in.btnLeft = true;
  updateSystem(st, in);
  assertEqual(MotorsTurnLeft, st.motors);

  in.btnLeft = false;
  in.btnBackward = true;
  updateSystem(st, in);
  assertEqual(MotorsBackward, st.motors);
}

void setup() {}
void loop() { TestRunner::run(); }

