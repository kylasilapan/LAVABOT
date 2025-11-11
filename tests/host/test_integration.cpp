#include <EpoxyDuino.h>
#include <AUnit.h>
#include "../../src/system_state.h"

using namespace aunit;

// Integration test: Complete sensor-to-actuator data flow
test(IntegrationSensorToActuatorFlow) {
  SystemState st;
  SystemInputs in = {};

  // Simulate PIR sensor detecting motion
  in.pirMotionDetected = true;
  updateSystem(st, in);
  
  // Verify relay (actuator) responds correctly - UV should be OFF
  assertFalse(st.relayOn);
  
  // Motion stops
  in.pirMotionDetected = false;
  updateSystem(st, in);
  
  // Relay should turn ON (UV can operate)
  assertTrue(st.relayOn);
}

// Integration test: RTC time integration with motor control
test(IntegrationRTCTimeToMotorControl) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;

  // RTC provides time data - before schedule window
  in.hour = 8;
  in.minute = 30;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);

  // RTC time enters schedule window
  in.hour = 9;
  in.minute = 15;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  // RTC time exits schedule window
  in.hour = 10;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
}

// Integration test: Button input to motor output in manual mode
test(IntegrationButtonToMotorOutput) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = false;

  // Forward button pressed -> Forward motor command
  in.btnForward = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  // Backward button pressed -> Backward motor command
  in.btnForward = false;
  in.btnBackward = true;
  updateSystem(st, in);
  assertEqual(MotorsBackward, st.motors);

  // Left button pressed -> Turn left motor command
  in.btnBackward = false;
  in.btnLeft = true;
  updateSystem(st, in);
  assertEqual(MotorsTurnLeft, st.motors);

  // Right button pressed -> Turn right motor command
  in.btnLeft = false;
  in.btnRight = true;
  updateSystem(st, in);
  assertEqual(MotorsTurnRight, st.motors);
}

// Integration test: Mode switch affects multiple subsystems
test(IntegrationModeSwitchAffectsSubsystems) {
  SystemState st;
  SystemInputs in = {};
  
  // Start in AUTO mode during schedule window
  st.autoMode = true;
  in.hour = 9;
  in.minute = 30;
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertTrue(st.autoMode);
  assertEqual(MotorsForward, st.motors);
  assertTrue(st.relayOn);

  // Switch to MANUAL mode
  in.modeButtonPressed = true;
  updateSystem(st, in);
  assertFalse(st.autoMode);
  assertEqual(MotorsStop, st.motors);  // Motors stop (no button pressed)
  assertTrue(st.relayOn);  // PIR still controls relay independently

  // Press forward in manual mode
  in.modeButtonPressed = false;
  in.btnForward = true;
  updateSystem(st, in);
  assertFalse(st.autoMode);
  assertEqual(MotorsForward, st.motors);
  assertTrue(st.relayOn);
}

// Integration test: Safety system integration (PIR + Relay + Motors)
test(IntegrationSafetySystemIntegration) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;
  in.hour = 9;
  in.minute = 30;  // In schedule window

  // Normal operation: no motion, motors can run, UV can operate
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertTrue(st.relayOn);  // UV ON
  assertEqual(MotorsForward, st.motors);  // Motors running

  // Safety trigger: motion detected
  in.pirMotionDetected = true;
  updateSystem(st, in);
  assertFalse(st.relayOn);  // UV OFF (safety)
  assertEqual(MotorsForward, st.motors);  // Motors can continue (optional safety feature)

  // Motion clears
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertTrue(st.relayOn);  // UV ON again
  assertEqual(MotorsForward, st.motors);  // Motors still running
}

// Integration test: End-to-end workflow - scheduled cleaning cycle
test(IntegrationScheduledCleaningCycle) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;
  in.pirMotionDetected = false;

  // Before cleaning window - system idle
  in.hour = 8;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
  assertTrue(st.relayOn);

  // Cleaning window starts - system activates
  in.hour = 9;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  assertTrue(st.relayOn);

  // During cleaning - motion detected (safety)
  in.pirMotionDetected = true;
  updateSystem(st, in);
  assertFalse(st.relayOn);  // UV off for safety
  assertEqual(MotorsForward, st.motors);  // Can continue moving

  // Motion clears - resume cleaning
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertTrue(st.relayOn);  // UV back on
  assertEqual(MotorsForward, st.motors);

  // Cleaning window ends - system deactivates
  in.hour = 10;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
  assertTrue(st.relayOn);
}

// Integration test: System state consistency across multiple updates
test(IntegrationStateConsistencyAcrossUpdates) {
  SystemState st;
  SystemInputs in = {};

  // Multiple rapid updates with varying inputs
  for (int i = 0; i < 100; i++) {
    in.hour = (i / 60) % 24;
    in.minute = i % 60;
    in.pirMotionDetected = (i % 3 == 0);
    in.modeButtonPressed = (i % 20 == 0);
    in.btnForward = (i % 7 == 0 && !st.autoMode);

    updateSystem(st, in);

    // Verify state is always valid
    assertTrue(st.autoMode == true || st.autoMode == false);
    assertTrue(st.relayOn == true || st.relayOn == false);
    assertTrue(st.motors >= MotorsStop && st.motors <= MotorsTurnRight);
    
    // Verify safety: if motion detected, relay must be off
    if (in.pirMotionDetected) {
      assertFalse(st.relayOn);
    }
  }
}

// Integration test: Concurrent system operations
test(IntegrationConcurrentSystemOperations) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;

  // Simulate concurrent operations: schedule active + motion detection
  in.hour = 9;
  in.minute = 30;  // In schedule
  in.pirMotionDetected = true;  // Motion detected
  updateSystem(st, in);

  // Motors should run (schedule active)
  assertEqual(MotorsForward, st.motors);
  // Relay should be off (safety override)
  assertFalse(st.relayOn);

  // Motion clears while still in schedule
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  assertTrue(st.relayOn);  // Relay can turn on
}

// Integration test: System response to invalid input combinations
test(IntegrationInvalidInputHandling) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;

  // Invalid time values should not crash system
  in.hour = -1;
  in.minute = -1;
  updateSystem(st, in);
  // System should handle gracefully - motors stop (outside valid window)
  assertEqual(MotorsStop, st.motors);

  // Extreme time values
  in.hour = 100;
  in.minute = 200;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);

  // All buttons pressed simultaneously (should use priority)
  st.autoMode = false;
  in.hour = 0;
  in.minute = 0;
  in.btnForward = true;
  in.btnBackward = true;
  in.btnLeft = true;
  in.btnRight = true;
  updateSystem(st, in);
  // Should select highest priority (Forward)
  assertEqual(MotorsForward, st.motors);
}

// setup() and loop() are defined in test_schedule.cpp to avoid multiple definitions

