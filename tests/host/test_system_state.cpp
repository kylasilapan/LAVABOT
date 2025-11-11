#include <EpoxyDuino.h>
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

test(ButtonPriorityOrder) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = false;

  // Forward has highest priority
  in.btnForward = true;
  in.btnBackward = true;
  in.btnLeft = true;
  in.btnRight = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  // Backward has priority over Left and Right
  in.btnForward = false;
  updateSystem(st, in);
  assertEqual(MotorsBackward, st.motors);

  // Left has priority over Right
  in.btnBackward = false;
  updateSystem(st, in);
  assertEqual(MotorsTurnLeft, st.motors);

  // Right is lowest priority
  in.btnLeft = false;
  updateSystem(st, in);
  assertEqual(MotorsTurnRight, st.motors);
}

test(ScheduleBoundaryTransitions) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;

  // Just before morning window
  in.hour = 8;
  in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);

  // Exact start of morning window
  in.hour = 9;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  // End of morning window
  in.hour = 9;
  in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  // Just after morning window
  in.hour = 10;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);

  // Just before afternoon window
  in.hour = 13;
  in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);

  // Exact start of afternoon window
  in.hour = 14;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  // End of afternoon window
  in.hour = 14;
  in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  // Just after afternoon window
  in.hour = 15;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
}

test(PIRBehaviorInBothModes) {
  SystemState st;
  SystemInputs in = {};

  // Test PIR in AUTO mode
  st.autoMode = true;
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertTrue(st.relayOn);

  in.pirMotionDetected = true;
  updateSystem(st, in);
  assertFalse(st.relayOn);

  // Test PIR in MANUAL mode
  st.autoMode = false;
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertTrue(st.relayOn);

  in.pirMotionDetected = true;
  updateSystem(st, in);
  assertFalse(st.relayOn);

  // PIR should work regardless of mode
  assertFalse(st.relayOn);
}

test(ManualButtonRelease) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = false;

  // Press forward button
  in.btnForward = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  // Release all buttons
  in.btnForward = false;
  in.btnBackward = false;
  in.btnLeft = false;
  in.btnRight = false;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);

  // Test with other buttons
  in.btnBackward = true;
  updateSystem(st, in);
  assertEqual(MotorsBackward, st.motors);

  in.btnBackward = false;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);

  in.btnRight = true;
  updateSystem(st, in);
  assertEqual(MotorsTurnRight, st.motors);

  in.btnRight = false;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
}

test(InitialStateCorrectness) {
  SystemState st;
  SystemInputs in = {};

  // Initial state should be AUTO mode, relay off, motors stopped
  assertTrue(st.autoMode);
  assertFalse(st.relayOn);
  assertEqual(MotorsStop, st.motors);

  // After updateSystem with no motion detected, relay should turn ON (UV safe to operate)
  // Default hour/minute (0,0) is outside schedule window, so motors stay stopped
  updateSystem(st, in);
  assertTrue(st.autoMode);  // Still in AUTO mode
  assertTrue(st.relayOn);   // Relay ON because no motion detected (UV can operate)
  assertEqual(MotorsStop, st.motors);  // Motors stopped (outside schedule window)
}

// setup() and loop() are defined in test_schedule.cpp to avoid multiple definitions

