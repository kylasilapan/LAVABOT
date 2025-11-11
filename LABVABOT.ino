#include <Wire.h>
#include "RTClib.h"
#include "src/schedule.h"
#include "src/system_state.h"

RTC_DS3231 rtc;

// PIR and relay
#define PIR_PIN      4
#define RELAY_PIN    5

// Motors
#define M1_IN1       18
#define M1_IN2       19
#define M2_IN1       20
#define M2_IN2       21

// Panel buttons
#define MODE_BTN     12
#define BTN_FWD      13
#define BTN_BWD      14
#define BTN_LEFT     15
#define BTN_RIGHT    16

// System state is now managed in SystemState struct

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    while (1);
  }

  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
  pinMode(M2_IN1, OUTPUT); pinMode(M2_IN2, OUTPUT);

  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(BTN_FWD, INPUT_PULLUP);
  pinMode(BTN_BWD, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  stopMotors();
}

void loop() {
  static SystemState st;
  static bool lastModeBtnState = HIGH;
  static unsigned long lastModeBtnTime = 0;

  // Read inputs
  SystemInputs inputs;
  
  // Mode button with debounce
  bool currentModeBtn = digitalRead(MODE_BTN);
  if (currentModeBtn == LOW && lastModeBtnState == HIGH && 
      (millis() - lastModeBtnTime) > 200) {
    inputs.modeButtonPressed = true;
    lastModeBtnTime = millis();
    Serial.println(st.autoMode ? "Switching to MANUAL Mode" : "Switching to AUTO Mode");
  } else {
    inputs.modeButtonPressed = false;
  }
  lastModeBtnState = currentModeBtn;

  inputs.pirMotionDetected = digitalRead(PIR_PIN) == HIGH;
  inputs.btnForward = digitalRead(BTN_FWD) == LOW;
  inputs.btnBackward = digitalRead(BTN_BWD) == LOW;
  inputs.btnLeft = digitalRead(BTN_LEFT) == LOW;
  inputs.btnRight = digitalRead(BTN_RIGHT) == LOW;

  // Get time from RTC
  DateTime now = rtc.now();
  inputs.hour = now.hour();
  inputs.minute = now.minute();

  // Update system state
  updateSystem(st, inputs);

  // Apply outputs to hardware
  digitalWrite(RELAY_PIN, st.relayOn ? HIGH : LOW);

  switch (st.motors) {
    case MotorsForward:
      driveForward();
      break;
    case MotorsBackward:
      driveBackward();
      break;
    case MotorsTurnLeft:
      turnLeft();
      break;
    case MotorsTurnRight:
      turnRight();
      break;
    case MotorsStop:
    default:
      stopMotors();
      break;
  }

  delay(100);
}

// Note: Auto schedule and manual control logic is now in src/system_state.h
// This allows the logic to be unit tested without hardware

// ----------------- Motor helpers -----------------
void driveForward() {
  digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, HIGH); digitalWrite(M2_IN2, LOW);
}

void driveBackward() {
  digitalWrite(M1_IN1, LOW);  digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN1, LOW);  digitalWrite(M2_IN2, HIGH);
}

void turnLeft() {
  digitalWrite(M1_IN1, LOW);  digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN1, HIGH); digitalWrite(M2_IN2, LOW);
}

void turnRight() {
  digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, LOW);  digitalWrite(M2_IN2, HIGH);
}

void stopMotors() {
  digitalWrite(M1_IN1, LOW);  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, LOW);  digitalWrite(M2_IN2, LOW);
}
