#include <Wire.h>
#include "RTClib.h"

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

bool autoMode = true;   // start in AUTO

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
  // --- Mode selection ---
  if (digitalRead(MODE_BTN) == LOW) {       // button pressed
    delay(200);                             // debounce
    autoMode = !autoMode;
    Serial.println(autoMode ? "AUTO Mode" : "MANUAL Mode");
    while (digitalRead(MODE_BTN) == LOW);   // wait for release
  }

  // --- PIR safety / UV ---
  int motion = digitalRead(PIR_PIN);
  if (motion == HIGH) {
    digitalWrite(RELAY_PIN, LOW);           // UV off if human detected
  } else {
    digitalWrite(RELAY_PIN, HIGH);          // UV on if no human
  }

  if (autoMode) {
    runAutoSchedule();
  } else {
    runManualControl();
  }

  delay(100);
}

// ----------------- AUTO Schedule -----------------
void runAutoSchedule() {
  DateTime now = rtc.now();
  int h = now.hour();
  int m = now.minute();

  bool inSchedule = ((h==9  && m>=0) || (h>9 && h<10)) ||
                    ((h==14 && m>=0) || (h>14 && h<15));

  if (inSchedule) {
    driveForward();
    Serial.println("AUTO: Moving");
  } else {
    stopMotors();
    Serial.println("AUTO: Idle");
  }
}

// ----------------- MANUAL Control -----------------
void runManualControl() {
  if (digitalRead(BTN_FWD) == LOW) driveForward();
  else if (digitalRead(BTN_BWD) == LOW) driveBackward();
  else if (digitalRead(BTN_LEFT) == LOW) turnLeft();
  else if (digitalRead(BTN_RIGHT) == LOW) turnRight();
  else stopMotors();
}

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
