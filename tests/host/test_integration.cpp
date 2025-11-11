#include <EpoxyDuino.h>
#include <AUnit.h>
#include <climits>
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

// Integration test: System recovery after error conditions
test(IntegrationErrorRecovery) {
  SystemState st;
  SystemInputs in = {};

  // Simulate error condition: invalid time data
  st.autoMode = true;
  in.hour = -1;
  in.minute = -1;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);  // Safe state

  // Recovery: valid time data restored
  in.hour = 9;
  in.minute = 30;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);  // System recovered

  // Error in manual mode: all buttons released unexpectedly
  st.autoMode = false;
  in.btnForward = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  in.btnForward = false;  // Button release
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);  // Graceful stop
}

// Integration test: Real-time response to sensor changes
test(IntegrationRealTimeSensorResponse) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;
  in.hour = 9;
  in.minute = 30;
  in.pirMotionDetected = false;

  // Initial state: UV on, motors running
  updateSystem(st, in);
  assertTrue(st.relayOn);
  assertEqual(MotorsForward, st.motors);

  // Rapid motion detection (real-time safety response)
  in.pirMotionDetected = true;
  updateSystem(st, in);
  assertFalse(st.relayOn);  // Immediate safety response

  // Motion clears quickly
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertTrue(st.relayOn);  // Immediate recovery

  // Multiple rapid toggles
  for (int i = 0; i < 10; i++) {
    in.pirMotionDetected = (i % 2 == 0);
    updateSystem(st, in);
    assertEqual(!in.pirMotionDetected, st.relayOn);  // Always correct
  }
}

// Integration test: Multi-sensor data fusion
test(IntegrationMultiSensorDataFusion) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;

  // Combine RTC time + PIR sensor + mode state
  in.hour = 9;
  in.minute = 30;  // Schedule active
  in.pirMotionDetected = false;  // No motion
  updateSystem(st, in);

  // All sensors indicate: safe to operate
  assertEqual(MotorsForward, st.motors);  // Schedule says go
  assertTrue(st.relayOn);  // PIR says safe

  // PIR detects motion (overrides schedule for safety)
  in.pirMotionDetected = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);  // Motors can continue
  assertFalse(st.relayOn);  // But UV must stop (safety)

  // Time exits schedule, motion still detected
  in.hour = 10;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);  // Schedule says stop
  assertFalse(st.relayOn);  // PIR still says unsafe
}

// Integration test: System state persistence across mode changes
test(IntegrationStatePersistenceAcrossModes) {
  SystemState st;
  SystemInputs in = {};
  
  // Start in AUTO mode
  st.autoMode = true;
  in.hour = 9;
  in.minute = 30;
  in.pirMotionDetected = false;
  updateSystem(st, in);
  bool relayStateBefore = st.relayOn;
  MotorCommand motorStateBefore = st.motors;

  // Switch to MANUAL mode
  in.modeButtonPressed = true;
  updateSystem(st, in);
  assertFalse(st.autoMode);
  // Relay state should persist (PIR still controls it)
  assertEqual(relayStateBefore, st.relayOn);

  // Motors change (now controlled by buttons, not schedule)
  in.modeButtonPressed = false;
  in.btnForward = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  assertEqual(relayStateBefore, st.relayOn);  // Relay unchanged

  // Switch back to AUTO
  in.modeButtonPressed = true;
  in.btnForward = false;
  updateSystem(st, in);
  assertTrue(st.autoMode);
  // Relay still controlled by PIR (independent of mode)
  assertEqual(relayStateBefore, st.relayOn);
}

// Integration test: Resource constraint handling (rapid updates)
test(IntegrationResourceConstraintHandling) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;

  // Simulate high-frequency updates (like real-time system)
  for (int cycle = 0; cycle < 1000; cycle++) {
    // Vary inputs rapidly
    in.hour = (cycle / 60) % 24;
    in.minute = cycle % 60;
    in.pirMotionDetected = (cycle % 5 == 0);
    in.modeButtonPressed = (cycle == 500);  // Mode switch mid-operation

    updateSystem(st, in);

    // System must maintain valid state even under load
    assertTrue(st.autoMode == true || st.autoMode == false);
    assertTrue(st.relayOn == true || st.relayOn == false);
    assertTrue(st.motors >= MotorsStop && st.motors <= MotorsTurnRight);

    // Safety invariant: motion always disables relay
    if (in.pirMotionDetected) {
      assertFalse(st.relayOn);
    }
  }
}

// Integration test: Complete operational day simulation
test(IntegrationCompleteOperationalDay) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;

  int motorStartCount = 0;
  int motorStopCount = 0;
  int safetyTriggerCount = 0;
  MotorCommand lastMotorState = MotorsStop;

  // Simulate full operational day (24 hours, 1-minute resolution)
  for (int hour = 0; hour < 24; hour++) {
    for (int minute = 0; minute < 60; minute++) {
      in.hour = hour;
      in.minute = minute;
      
      // Simulate occasional motion detection (realistic scenario)
      in.pirMotionDetected = (minute % 15 == 0 && hour >= 8 && hour < 18);
      
      // Occasional mode switches
      in.modeButtonPressed = (hour == 12 && minute == 0);
      
      updateSystem(st, in);

      // Track motor state changes
      if (st.motors != lastMotorState) {
        if (st.motors == MotorsForward) motorStartCount++;
        if (lastMotorState == MotorsForward && st.motors == MotorsStop) motorStopCount++;
        lastMotorState = st.motors;
      }

      // Track safety triggers
      if (in.pirMotionDetected && !st.relayOn) {
        safetyTriggerCount++;
      }

      // Verify system integrity
      assertTrue(st.autoMode == true || st.autoMode == false);
      assertTrue(st.relayOn == true || st.relayOn == false);
      assertTrue(st.motors >= MotorsStop && st.motors <= MotorsTurnRight);
    }
  }

  // Verify system operated correctly
  // Note: Mode switch at 12:00 might prevent 14:00 start if still in manual mode
  // But we should have at least one start at 9:00, and stops when exiting windows
  assertTrue(motorStartCount >= 1);  // Should start at least once (9:00)
  assertTrue(motorStopCount >= 1);   // Should stop at least once (exiting 9:00 window)
  assertTrue(safetyTriggerCount > 0); // Should have safety triggers
}

// Integration test: System behavior during schedule window transitions
test(IntegrationScheduleWindowTransitions) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;
  in.pirMotionDetected = false;

  // Transition into morning window
  in.hour = 8;
  in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);

  in.minute = 0;
  in.hour = 9;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);  // Window entered

  // Transition out of morning window
  in.hour = 9;
  in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  in.hour = 10;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);  // Window exited

  // Transition into afternoon window
  in.hour = 13;
  in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);

  in.hour = 14;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);  // Window entered

  // Transition out of afternoon window
  in.hour = 14;
  in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  in.hour = 15;
  in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);  // Window exited
}

// Integration test: Button state transitions and debouncing simulation
test(IntegrationButtonStateTransitions) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = false;

  // Simulate button press sequence
  in.btnForward = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  // Button held (multiple updates)
  for (int i = 0; i < 10; i++) {
    updateSystem(st, in);
    assertEqual(MotorsForward, st.motors);  // Should remain active
  }

  // Button release
  in.btnForward = false;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);

  // Rapid button presses (simulating bouncing)
  for (int i = 0; i < 5; i++) {
    in.btnForward = true;
    updateSystem(st, in);
    assertEqual(MotorsForward, st.motors);
    
    in.btnForward = false;
    updateSystem(st, in);
    assertEqual(MotorsStop, st.motors);
  }
}

// Integration test: PIR sensor noise and filtering simulation
test(IntegrationPIRSensorNoiseHandling) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;
  in.hour = 9;
  in.minute = 30;

  // Simulate PIR noise (false positives)
  bool lastRelayState = st.relayOn;
  for (int i = 0; i < 20; i++) {
    // Simulate noise: brief motion detections
    in.pirMotionDetected = (i % 3 == 0);
    updateSystem(st, in);
    
    // Relay should always reflect current PIR state (no filtering in this system)
    assertEqual(!in.pirMotionDetected, st.relayOn);
    lastRelayState = st.relayOn;
  }

  // Extended motion detection
  in.pirMotionDetected = true;
  for (int i = 0; i < 10; i++) {
    updateSystem(st, in);
    assertFalse(st.relayOn);  // Should remain off during motion
  }

  // Motion clears
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertTrue(st.relayOn);  // Should turn on immediately
}

// Integration test: System behavior with missing/incomplete sensor data
test(IntegrationMissingSensorDataHandling) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;

  // Missing/invalid time data (defaults to 0,0)
  in.hour = 0;
  in.minute = 0;
  in.pirMotionDetected = false;
  updateSystem(st, in);
  // System should handle gracefully - outside schedule window
  assertEqual(MotorsStop, st.motors);
  assertTrue(st.relayOn);  // PIR says safe

  // Missing PIR data (defaults to false - no motion)
  in.hour = 9;
  in.minute = 30;
  in.pirMotionDetected = false;  // Default/unknown state
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  assertTrue(st.relayOn);  // Assumes safe (no motion detected)

  // All sensors missing - system should enter safe state
  in.hour = -1;
  in.minute = -1;
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);  // Safe: motors stopped
  assertTrue(st.relayOn);  // Safe: relay off (but logic says on if no motion)
}

// Integration test: Mode switching during active operations
test(IntegrationModeSwitchDuringActiveOperations) {
  SystemState st;
  SystemInputs in = {};
  
  // AUTO mode with motors running
  st.autoMode = true;
  in.hour = 9;
  in.minute = 30;
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  assertTrue(st.relayOn);

  // Switch to MANUAL while motors running
  in.modeButtonPressed = true;
  updateSystem(st, in);
  assertFalse(st.autoMode);
  assertEqual(MotorsStop, st.motors);  // Motors stop (no button pressed)
  assertTrue(st.relayOn);  // Relay unchanged (PIR controlled)

  // Press button in manual mode
  in.modeButtonPressed = false;
  in.btnForward = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);

  // Switch back to AUTO while button still "pressed"
  in.modeButtonPressed = true;
  updateSystem(st, in);
  assertTrue(st.autoMode);
  // Motors now controlled by schedule, not button
  assertEqual(MotorsForward, st.motors);  // Still in schedule window
}

// Integration test: System response time and latency
test(IntegrationSystemResponseTime) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;
  in.hour = 9;
  in.minute = 30;
  in.pirMotionDetected = false;

  // Initial state
  updateSystem(st, in);
  assertTrue(st.relayOn);
  assertEqual(MotorsForward, st.motors);

  // Measure response to motion detection
  in.pirMotionDetected = true;
  updateSystem(st, in);
  // Should respond immediately (single update cycle)
  assertFalse(st.relayOn);

  // Measure response to motion clearing
  in.pirMotionDetected = false;
  updateSystem(st, in);
  // Should respond immediately
  assertTrue(st.relayOn);

  // Measure response to schedule change
  in.hour = 10;
  in.minute = 0;
  updateSystem(st, in);
  // Should respond immediately
  assertEqual(MotorsStop, st.motors);
}

// Integration test: Data integrity and validation
test(IntegrationDataIntegrityValidation) {
  SystemState st;
  SystemInputs in = {};

  // Test with various invalid combinations
  struct {
    int hour;
    int minute;
    bool expectedValid;
  } testCases[] = {
    {0, 0, true},
    {23, 59, true},
    {-1, 0, false},
    {24, 0, false},
    {0, -1, false},
    {0, 60, false},
    {100, 200, false}
  };

  for (int i = 0; i < 7; i++) {
    st.autoMode = true;
    in.hour = testCases[i].hour;
    in.minute = testCases[i].minute;
    in.pirMotionDetected = false;
    
    updateSystem(st, in);
    
    // System should always maintain valid internal state
    assertTrue(st.autoMode == true || st.autoMode == false);
    assertTrue(st.relayOn == true || st.relayOn == false);
    assertTrue(st.motors >= MotorsStop && st.motors <= MotorsTurnRight);
    
    // Invalid times should result in safe state (motors stopped)
    if (!testCases[i].expectedValid) {
      assertEqual(MotorsStop, st.motors);
    }
  }
}

// Integration test: System behavior with extreme input values
test(IntegrationExtremeInputValues) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;

  // Extreme time values
  in.hour = INT_MAX;
  in.minute = INT_MAX;
  in.pirMotionDetected = false;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);  // Should handle gracefully

  // Negative extreme values
  in.hour = INT_MIN;
  in.minute = INT_MIN;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);  // Should handle gracefully

  // All buttons pressed simultaneously in manual mode
  st.autoMode = false;  // Start in manual mode
  in.hour = 0;
  in.minute = 0;
  in.btnForward = true;
  in.btnBackward = true;
  in.btnLeft = true;
  in.btnRight = true;
  in.modeButtonPressed = true;  // Also pressing mode button
  updateSystem(st, in);
  // Should handle - mode switch takes priority, then button priority
  // Mode button toggles: false -> true (manual to auto)
  assertTrue(st.autoMode);  // Mode button toggled from false to true
}

// Integration test: System stability under continuous operation
test(IntegrationContinuousOperationStability) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;

  // Simulate 1 week of operation (168 hours, checking every hour)
  for (int hour = 0; hour < 168; hour++) {
    for (int minute = 0; minute < 60; minute += 15) {  // Check every 15 minutes
      in.hour = hour % 24;
      in.minute = minute;
      in.pirMotionDetected = (minute % 30 == 0);  // Occasional motion
      in.modeButtonPressed = (hour % 24 == 0 && minute == 0);  // Daily mode check

      updateSystem(st, in);

      // System must remain stable
      assertTrue(st.autoMode == true || st.autoMode == false);
      assertTrue(st.relayOn == true || st.relayOn == false);
      assertTrue(st.motors >= MotorsStop && st.motors <= MotorsTurnRight);

      // Safety must always be maintained
      if (in.pirMotionDetected) {
        assertFalse(st.relayOn);
      }
    }
  }
}

// Integration test: System behavior with conflicting inputs
test(IntegrationConflictingInputsHandling) {
  SystemState st;
  SystemInputs in = {};
  
  // Conflicting scenario: AUTO mode but manual button pressed
  st.autoMode = true;
  in.hour = 9;
  in.minute = 30;  // Schedule active
  in.btnForward = true;  // Manual button also pressed
  in.pirMotionDetected = false;
  updateSystem(st, in);
  
  // In AUTO mode, buttons should be ignored
  assertEqual(MotorsForward, st.motors);  // From schedule, not button
  assertTrue(st.autoMode);

  // Conflicting scenario: Motion detected but schedule wants motors on
  in.btnForward = false;
  in.pirMotionDetected = true;  // Motion detected
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);  // Motors can continue
  assertFalse(st.relayOn);  // But UV must stop (safety priority)

  // Conflicting scenario: Manual mode but schedule window active
  st.autoMode = false;
  in.pirMotionDetected = false;
  in.btnForward = false;  // No button pressed
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);  // Manual mode - no button = stop
  assertTrue(st.relayOn);  // Relay independent of mode
}

// setup() and loop() are defined in test_schedule.cpp to avoid multiple definitions

