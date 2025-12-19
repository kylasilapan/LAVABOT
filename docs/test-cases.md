# LAVABOT — Test Cases Derived From Firmware (ESP8266 Sketch)

Source:
- Firmware file (provided): main ESP8266 sketch (web server, motor & relay control, IR sensors, scheduler)
- Endpoints & behavior references are taken from the code you supplied (HTTP handlers, motor/relay functions, autoNavigate(), scheduler, etc.)

---

Test Case ID
TC-001

Test Case Name
Toggle Automatic Mode ON

Module Name
Web API / Auto Mode (handleToggleAuto)

Precondition
Device powered on; emergencyStop == false.

Test Steps
1. Send HTTP GET to /toggle_auto.
2. Observe response and device behavior.

Test Data
None.

Expected Result
- HTTP 200 with body containing "Automatic Mode ON".
- autoMode becomes true.
- uvStartTime set to millis() (UV lamp ON).
- Relay1 (UV) set HIGH, Relay2 set LOW.
- isMoving and waitingAtPosition are false; nextDirection == FORWARD.

Actual Result
- As per code: server sends 200 "Automatic Mode ON", uvStartTime = millis(), digitalWrite(RELAY1_PIN, HIGH), digitalWrite(RELAY2_PIN, LOW), autoMode true.

Status
Ready to execute.

---

Test Case ID
TC-002

Test Case Name
Toggle Automatic Mode OFF (return to manual)

Module Name
Web API / Auto Mode (handleToggleAuto)

Precondition
autoMode == true, emergencyStop == false.

Test Steps
1. Send HTTP GET to /toggle_auto.
2. Observe response and device behavior.

Test Data
None.

Expected Result
- HTTP 200 with body containing "Manual Mode ON".
- autoMode becomes false.
- All motors stopped.
- Relay1 and Relay2 set LOW.
- isMoving and waitingAtPosition become false; uvCycleComplete false.

Actual Result
- As per code: server sends 200 "Manual Mode ON"; stopMotors() executed; relays set LOW; flags reset.

Status
Ready to execute.

---

Test Case ID
TC-003

Test Case Name
Emergency Stop Activation via Web

Module Name
Web API / Emergency Stop (handleEmergencyStop)

Precondition
Device may be in any mode (manual/auto). emergencyStop == false.

Test Steps
1. Send HTTP GET to /emergency_stop.
2. Observe response and device state.

Test Data
None.

Expected Result
- HTTP 200 with "Emergency Stop Activated".
- emergencyStop becomes true.
- autoMode becomes false.
- stopMotors() called (motors PWM = 0).
- Both relays set LOW.
- isMoving and waitingAtPosition set false.
- uvCycleComplete set false.

Actual Result
- As per code: server responds with "Emergency Stop Activated" and printed serial logs; relays off; motors stopped; flags set.

Status
Ready to execute.

---

Test Case ID
TC-004

Test Case Name
Reset Emergency via Web

Module Name
Web API / Reset Emergency (handleResetEmergency)

Precondition
emergencyStop == true.

Test Steps
1. Send HTTP GET to /reset_emergency.
2. Observe response and device state.

Test Data
None.

Expected Result
- HTTP 200 with "System Reset".
- emergencyStop becomes false (system allows normal operation).
- No autoMode change in code (remains false unless toggled).

Actual Result
- As per code: server sends "System Reset"; emergencyStop set false; Serial prints reset message.

Status
Ready to execute.

---

Test Case ID
TC-005

Test Case Name
UV Cycle Completion (30 minutes)

Module Name
Auto Mode / UV Timer (main loop + handleTimerStatus)

Precondition
autoMode == true, uvCycleComplete == false, uvStartTime set.

Test Steps
1. Start auto mode (toggle_auto).
2. Simulate passage of UV_DURATION (30 minutes) OR mock millis() to meet condition.
3. Observe behavior when millis() - uvStartTime >= UV_DURATION.

Test Data
- UV_DURATION = 30 * 60 * 1000

Expected Result
- Relay1 (UV) set LOW.
- Relay2 set HIGH.
- uvCycleComplete becomes true.
- stopMotors() called; isMoving = false; waitingAtPosition = false.
- Serial logs show "✓ UV Sterilization Complete!".
- handleTimerStatus returns active=false after completion.

Actual Result
- As per code: relays toggled, motors stopped, uvCycleComplete set true.

Status
Requires timed test or simulation (can be accelerated via mocking time or reducing UV_DURATION locally).

---

Test Case ID
TC-006

Test Case Name
Movement Cycle — Start and 5sec Move

Module Name
autoNavigate / Movement

Precondition
autoMode == true, uvCycleComplete == false, not moving, not waiting.

Test Steps
1. Ensure no obstacles (IR sensors read HIGH).
2. Execute autoNavigate() (implicitly run by loop()).
3. Verify motors start and isMoving true, moveStartTime set.

Test Data
- MOVE_DURATION = 5000 ms.

Expected Result
- isMoving set true.
- moveStartTime = millis() at start.
- Motors set to chosen safe direction (FORWARD if front clear).
- Serial logs indicate starting movement for 5 sec.

Actual Result
- As per code: findSafeDirection() chosen, forwardMotors() or other executes, isMoving true.

Status
Ready to execute; requires IR sensor simulation or wiring.

---

Test Case ID
TC-007

Test Case Name
Movement Collision During 5sec — Immediate Direction Change

Module Name
autoNavigate / IR detection

Precondition
isMoving == true; currentState == FORWARD; during move, front IR becomes LOW (obstacle).

Test Steps
1. Begin movement forward in auto mode.
2. Simulate front obstacle by setting IR_FRONT_LEFT or IR_FRONT_RIGHT to LOW.
3. Observe action.

Test Data
None.

Expected Result
- Code detects wallDetected true.
- stopMotors() called, brief delay, findSafeDirection() returns another direction.
- Motors start in new direction and moveStartTime reset.
- Serial prints wall detection and direction change.

Actual Result
- As per code: wall detection branches executed and new direction chosen (or RIGHT if cornered).

Status
Ready to execute; requires IR simulation.

---

Test Case ID
TC-008

Test Case Name
Movement Wait Completion (20 minutes wait -> resume movement)

Module Name
autoNavigate / Position wait

Precondition
waitingAtPosition == true; positionStartTime set.

Test Steps
1. Enter waitingAtPosition state (after 5-sec movement ended).
2. Simulate passage of POSITION_WAIT_TIME (20 minutes) or mock millis().
3. Observe resumed movement.

Test Data
- POSITION_WAIT_TIME = 20 * 60 * 1000

Expected Result
- After elapsed >= POSITION_WAIT_TIME: waitingAtPosition false; isMoving true; moveStartTime set; new direction chosen via findSafeDirection(); motors start for 5 sec.

Actual Result
- As per code: flags toggled and movement restarted.

Status
Requires timed test or simulation.

---

Test Case ID
TC-009

Test Case Name
Manual Movement Commands (Forward / Backward / Left / Right / Stop)

Module Name
Manual Control Handlers (handleForward, handleBackward, handleLeft, handleRight, handleStop)

Precondition
emergencyStop == false.

Test Steps
1. Send HTTP GET to /forward, /backward, /left, /right, and /stop.
2. Observe HTTP responses and motor behavior.

Test Data
None.

Expected Result
- /forward -> HTTP 200 "Moving Forward" and forwardMotors() invoked.
- /backward -> HTTP 200 "Moving Backward" and backwardMotors() invoked.
- /left -> HTTP 200 "Turning Left" and leftMotors() invoked.
- /right -> HTTP 200 "Turning Right" and rightMotors() invoked.
- /stop -> HTTP 200 "Stopped" and stopMotors() invoked.

Actual Result
- As per code: server endpoints call the respective handlers and motors are driven accordingly (unless emergencyStop is active, which returns 400).

Status
Ready to execute.

---

Test Case ID
TC-010

Test Case Name
Relay 1 (UV) Manual ON / OFF

Module Name
Relay Control (relay1On, relay1Off)

Precondition
emergencyStop == false for ON case.

Test Steps
1. Send HTTP GET to /relay1_on.
2. Verify response and that RELAY1_PIN is HIGH.
3. Send HTTP GET to /relay1_off.
4. Verify response and RELAY1_PIN is LOW.

Test Data
None.

Expected Result
- /relay1_on -> HTTP 200 "UV Light ON"; digitalWrite(RELAY1_PIN, HIGH).
- /relay1_off -> HTTP 200 "UV Light OFF"; digitalWrite(RELAY1_PIN, LOW).

Actual Result
- As per code: relay toggled; serial logs printed. If emergencyStop active, /relay1_on returns 400.

Status
Ready to execute.

---

Test Case ID
TC-011

Test Case Name
Relay 2 Manual ON / OFF (and auto activation when UV complete)

Module Name
Relay Control (relay2On, relay2Off) & UV completion

Precondition
For manual ON: emergencyStop == false.

Test Steps
1. Send HTTP GET to /relay2_on then /relay2_off and observe.
2. For auto activation: complete UV cycle (TC-005) and observe Relay2 becomes HIGH.

Test Data
None.

Expected Result
- /relay2_on -> HTTP 200 "Relay 2 ON"; RELAY2_PIN HIGH.
- /relay2_off -> HTTP 200 "Relay 2 OFF"; RELAY2_PIN LOW.
- After UV completion, Relay2 set HIGH automatically.

Actual Result
- As per code.

Status
Ready to execute.

---

Test Case ID
TC-012

Test Case Name
IR Status Endpoint JSON Format

Module Name
IR Sensor Status (handleIRStatus)

Precondition
Device running.

Test Steps
1. Send HTTP GET to /ir_status.
2. Inspect returned JSON.

Test Data
None.

Expected Result
- HTTP 200 response with JSON:
  {"frontLeft":<0|1>,"frontRight":<0|1>,"backLeft":<0|1>,"backRight":<0|1>}
- Values reflect digitalRead(sensorPin) == LOW -> obstacle = 1 (true).

Actual Result
- As per code: JSON constructed using String(checkObstacle(...)).

Status
Ready to execute.

---

Test Case ID
TC-013

Test Case Name
Timer Status Endpoint JSON Format

Module Name
UV Timer Status (handleTimerStatus)

Precondition
Device running.

Test Steps
1. Send HTTP GET to /timer_status.
2. Inspect returned JSON.

Test Data
None.

Expected Result
- If autoMode && !uvCycleComplete:
  {"active":true,"remaining":"MM:SS"}
- Otherwise:
  {"active":false,"remaining":"--:--"}

Actual Result
- As per code: JSON created accordingly.

Status
Ready to execute.

---

Test Case ID
TC-014

Test Case Name
Movement Status Endpoint JSON Format

Module Name
Movement Status (handleMovementStatus)

Precondition
Device running.

Test Steps
1. Send HTTP GET to /movement_status during different states: moving, waitingAtPosition, stopped.
2. Inspect returned JSON.

Test Data
None.

Expected Result
- Moving: {"status":"moving","remaining":"Ns"} where N = seconds left from MOVE_DURATION.
- Waiting: {"status":"waiting","remaining":"MM:SS"} where time left in position wait.
- Stopped: {"status":"stopped","remaining":"--:--"}

Actual Result
- As per code.

Status
Ready to execute.

---

Test Case ID
TC-015

Test Case Name
Scheduling Auto Mode (Set Schedule and Trigger)

Module Name
Scheduler (handleSetSchedule + main loop check)

Precondition
Device running; emergencyStop == false.

Test Steps
1. Send HTTP GET to /setSchedule?datetime=YYYY-MM-DDTHH:MM with a future datetime.
2. Verify response and scheduleSet true, scheduledTime set.
3. Simulate time advancement to scheduledTime (or adjust system time).
4. Verify autoMode becomes true, uvStartTime set, Relay1 HIGH.

Test Data
- datetime parameter in "%Y-%m-%dT%H:%M" format.

Expected Result
- /setSchedule responds 200 "Auto mode scheduled for <datetime>".
- scheduledTime stored (mktime from parsed tm).
- When time(nullptr) >= scheduledTime: autoMode true, uvStartTime millis(), relays set as in TC-001.

Actual Result
- As per code.

Status
Requires scheduler/time simulation or NTP/time adjustment.

---

Test Case ID
TC-016

Test Case Name
Toggle Auto when Emergency Active (should be prevented)

Module Name
handleToggleAuto & Emergency logic

Precondition
emergencyStop == true.

Test Steps
1. Ensure emergencyStop true (use /emergency_stop).
2. Send HTTP GET to /toggle_auto.

Test Data
None.

Expected Result
- server responds with HTTP 400 and body: "Cannot start - Emergency stop active. Reset first."
- autoMode remains false.

Actual Result
- As per code: the handler checks emergencyStop and sends 400.

Status
Ready to execute.

---

Test Case ID
TC-017

Test Case Name
Attempt Scheduling while Emergency Active (prevent)

Module Name
handleSetSchedule & Emergency logic

Precondition
emergencyStop == true.

Test Steps
1. Use /emergency_stop to activate emergency.
2. Send /setSchedule?datetime=...

Test Data
Valid datetime string.

Expected Result
- HTTP 400 with "Cannot schedule - Emergency stop active".
- scheduleSet remains unchanged.

Actual Result
- As per code.

Status
Ready to execute.

---

Test Case ID
TC-018

Test Case Name
Web UI Served at Root

Module Name
HTTP Server (server.on("/"))

Precondition
Device connected to WiFi; server started.

Test Steps
1. Browse to http://<device-ip>/. 
2. Verify HTML page loads and contains expected UI elements.

Test Data
None.

Expected Result
- HTTP 200 and HTML page returned (htmlPage()) with UI content and JS endpoints referencing /timer_status, /movement_status, etc.

Actual Result
- As per code: server.send(200, "text/html", htmlPage()).

Status
Ready to execute (requires WiFi connectivity).

---

Footer
- These test cases cover API-level, behavioral, and state transitions implemented in the firmware.
- The repository unit tests (tests/test_lavabot_logic.cpp) cover a small subset of logic (startAutoMode, triggerEmergencyStop, resetEmergency, finishUvCycle). The tests above expand coverage to the runtime firmware behaviors and endpoints.
- For automated verification:
  - Unit tests may be created for logic functions if you refactor the firmware logic into testable modules (e.g., separate logic from hardware reads/writes and mock hardware).
  - Integration tests can exercise endpoints by issuing HTTP requests to a running device or an emulator.
- If you want, I can:
  - Commit this file to docs/test-cases.md in the repo,
  - Create automated test harness code (hosted as unit tests or integration tests) to exercise many of these cases,
  - Or generate a short test-run checklist / script (curl commands and required hardware simulation steps).
