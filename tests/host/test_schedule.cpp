#include <AUnit.h>
#include "../../src/schedule.h"

using namespace aunit;

test(InsideMorning) {
  assertTrue(inAutoWindow(9, 0));
  assertTrue(inAutoWindow(9, 59));
}

test(InsideAfternoon) {
  assertTrue(inAutoWindow(14, 0));
  assertTrue(inAutoWindow(14, 59));
}

test(OutsideWindows) {
  assertFalse(inAutoWindow(8, 59));
  assertFalse(inAutoWindow(10, 0));
  assertFalse(inAutoWindow(13, 59));
  assertFalse(inAutoWindow(15, 0));
}

test(EdgeCaseBoundaryTimes) {
  // Test exact boundaries
  assertFalse(inAutoWindow(8, 59));
  assertTrue(inAutoWindow(9, 0));
  assertTrue(inAutoWindow(9, 59));
  assertFalse(inAutoWindow(10, 0));
  
  assertFalse(inAutoWindow(13, 59));
  assertTrue(inAutoWindow(14, 0));
  assertTrue(inAutoWindow(14, 59));
  assertFalse(inAutoWindow(15, 0));
}

test(EdgeCaseAllHours) {
  // Test all 24 hours
  for (int h = 0; h < 24; h++) {
    for (int m = 0; m < 60; m += 15) { // Test every 15 minutes
      bool expected = (h == 9 || h == 14);
      assertEqual(expected, inAutoWindow(h, m));
    }
  }
}

test(EdgeCaseInvalidTimes) {
  // Test invalid/edge values - should not crash
  assertFalse(inAutoWindow(-1, -1));
  assertFalse(inAutoWindow(25, 0));
  assertFalse(inAutoWindow(9, 100));
  assertFalse(inAutoWindow(100, 200));
}

void setup() {}
void loop() { TestRunner::run(); }

