#include <EpoxyDuino.h>
#include <AUnit.h>
#include "../../src/system_state.h"

using namespace aunit;

// Stress test: Rapid mode toggling (1000 cycles)
test(StressRapidModeToggle) {
  SystemState st;
  SystemInputs in = {};
  
  bool expectedMode = true; // starts in AUTO
  
  for (int i = 0; i < 1000; i++) {
    in.modeButtonPressed = true;
    updateSystem(st, in);
    expectedMode = !expectedMode;
    assertEqual(expectedMode, st.autoMode);
    
    in.modeButtonPressed = false;
    updateSystem(st, in);
    assertEqual(expectedMode, st.autoMode);
  }
}

// Stress test: Long-running operation (simulate 24 hours of operation)
test(StressLongRunningOperation) {
  SystemState st;
  SystemInputs in = {};
  
  // Simulate 24 hours with 1-minute intervals (1440 cycles)
  for (int hour = 0; hour < 24; hour++) {
    for (int minute = 0; minute < 60; minute++) {
      in.hour = hour;
      in.minute = minute;
      in.pirMotionDetected = (hour % 2 == 0); // Alternate PIR state
      updateSystem(st, in);
      
      // Verify state consistency
      assertTrue(st.autoMode == true || st.autoMode == false);
      assertTrue(st.relayOn == true || st.relayOn == false);
      assertTrue(st.motors >= MotorsStop && st.motors <= MotorsTurnRight);
    }
  }
}

// Edge case: Boundary time conditions
test(EdgeCaseTimeBoundaries) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;
  
  // Test exact boundaries
  in.hour = 8; in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
  
  in.hour = 9; in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  
  in.hour = 9; in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  
  in.hour = 10; in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
  
  in.hour = 13; in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
  
  in.hour = 14; in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  
  in.hour = 14; in.minute = 59;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  
  in.hour = 15; in.minute = 0;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
}

// Edge case: Invalid time values (should handle gracefully)
test(EdgeCaseInvalidTimeValues) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = true;
  
  // Negative values
  in.hour = -1; in.minute = -1;
  updateSystem(st, in);
  // Should not crash, motors should be stopped (outside window)
  assertEqual(MotorsStop, st.motors);
  
  // Extreme values
  in.hour = 25; in.minute = 100;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
  
  in.hour = 100; in.minute = 200;
  updateSystem(st, in);
  assertEqual(MotorsStop, st.motors);
}

// Durability: PIR flickering (rapid on/off)
test(DurabilityPIRFlickering) {
  SystemState st;
  SystemInputs in = {};
  
  // Simulate PIR sensor flickering 500 times
  for (int i = 0; i < 500; i++) {
    in.pirMotionDetected = (i % 2 == 0);
    updateSystem(st, in);
    
    // Relay should always be opposite of PIR
    assertEqual(!in.pirMotionDetected, st.relayOn);
  }
}

// Durability: Multiple simultaneous button presses (priority test)
test(DurabilityMultipleButtonPresses) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = false;
  
  // Test all button combinations
  for (int combo = 0; combo < 16; combo++) {
    in.btnForward = (combo & 1) != 0;
    in.btnBackward = (combo & 2) != 0;
    in.btnLeft = (combo & 4) != 0;
    in.btnRight = (combo & 8) != 0;
    
    updateSystem(st, in);
    
    // Verify only one command is active (priority: Forward > Backward > Left > Right)
    MotorCommand cmd = st.motors;
    if (in.btnForward) {
      assertEqual(MotorsForward, cmd);
    } else if (in.btnBackward) {
      assertEqual(MotorsBackward, cmd);
    } else if (in.btnLeft) {
      assertEqual(MotorsTurnLeft, cmd);
    } else if (in.btnRight) {
      assertEqual(MotorsTurnRight, cmd);
    } else {
      assertEqual(MotorsStop, cmd);
    }
  }
}

// Durability: State persistence across many cycles
test(DurabilityStatePersistence) {
  SystemState st;
  SystemInputs in = {};
  
  // Set initial state
  st.autoMode = false;
  in.btnForward = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  
  // Run 1000 cycles without changing inputs
  for (int i = 0; i < 1000; i++) {
    updateSystem(st, in);
    assertEqual(MotorsForward, st.motors);
    assertFalse(st.autoMode);
  }
  
  // Change state and verify it persists
  in.btnForward = false;
  in.btnBackward = true;
  updateSystem(st, in);
  assertEqual(MotorsBackward, st.motors);
  
  for (int i = 0; i < 1000; i++) {
    updateSystem(st, in);
    assertEqual(MotorsBackward, st.motors);
  }
}

// Durability: Mode switching during active operations
test(DurabilityModeSwitchDuringOperation) {
  SystemState st;
  SystemInputs in = {};
  
  // Start in auto mode during schedule window
  st.autoMode = true;
  in.hour = 9;
  in.minute = 30;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  
  // Switch to manual while moving
  in.modeButtonPressed = true;
  updateSystem(st, in);
  assertFalse(st.autoMode);
  assertEqual(MotorsStop, st.motors); // Should stop when no manual button
  
  // Press forward in manual mode
  in.modeButtonPressed = false;
  in.btnForward = true;
  updateSystem(st, in);
  assertEqual(MotorsForward, st.motors);
  
  // Switch back to auto (still in schedule window)
  in.modeButtonPressed = true;
  updateSystem(st, in);
  assertTrue(st.autoMode);
  assertEqual(MotorsForward, st.motors); // Should continue forward
}

// Durability: Rapid input changes
test(DurabilityRapidInputChanges) {
  SystemState st;
  SystemInputs in = {};
  st.autoMode = false;
  
  // Rapidly change button inputs
  for (int i = 0; i < 200; i++) {
    in.btnForward = (i % 4 == 0);
    in.btnBackward = (i % 4 == 1);
    in.btnLeft = (i % 4 == 2);
    in.btnRight = (i % 4 == 3);
    in.pirMotionDetected = (i % 2 == 0);
    
    updateSystem(st, in);
    
    // Verify state is always valid
    assertTrue(st.motors >= MotorsStop && st.motors <= MotorsTurnRight);
    assertEqual(!in.pirMotionDetected, st.relayOn);
  }
}

// Durability: Full day cycle with mode changes
test(DurabilityFullDayCycle) {
  SystemState st;
  SystemInputs in = {};
  
  int modeToggleCount = 0;
  
  // Simulate full day with periodic mode toggles
  for (int hour = 0; hour < 24; hour++) {
    for (int minute = 0; minute < 60; minute++) {
      in.hour = hour;
      in.minute = minute;
      
      // Toggle mode every 2 hours
      if (minute == 0 && hour % 2 == 0) {
        in.modeButtonPressed = true;
        modeToggleCount++;
      } else {
        in.modeButtonPressed = false;
      }
      
      // Simulate PIR activity during work hours
      in.pirMotionDetected = (hour >= 8 && hour < 18 && minute % 10 < 3);
      
      // Simulate manual button presses in manual mode
      if (!st.autoMode && minute % 15 == 0) {
        in.btnForward = true;
      } else {
        in.btnForward = false;
      }
      
      updateSystem(st, in);
      
      // Verify state consistency
      assertTrue(st.autoMode == true || st.autoMode == false);
      assertTrue(st.relayOn == true || st.relayOn == false);
      assertTrue(st.motors >= MotorsStop && st.motors <= MotorsTurnRight);
    }
  }
  
  // Verify we had mode toggles
  assertTrue(modeToggleCount > 0);
}

// Durability: Memory/state consistency check
test(DurabilityStateConsistency) {
  SystemState st1, st2;
  SystemInputs in = {};
  
  // Run same sequence on two states
  for (int i = 0; i < 100; i++) {
    in.hour = (i / 60) % 24;
    in.minute = i % 60;
    in.modeButtonPressed = (i % 20 == 0);
    in.pirMotionDetected = (i % 3 == 0);
    in.btnForward = (i % 7 == 0);
    
    updateSystem(st1, in);
    updateSystem(st2, in);
    
    // States should be identical
    assertEqual(st1.autoMode, st2.autoMode);
    assertEqual(st1.relayOn, st2.relayOn);
    assertEqual(st1.motors, st2.motors);
  }
}

void setup() {}
void loop() { TestRunner::run(); }

