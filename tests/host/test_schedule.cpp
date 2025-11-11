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

void setup() {}
void loop() { TestRunner::run(); }

