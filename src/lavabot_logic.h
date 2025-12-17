#ifndef LAVABOT_LOGIC_H
#define LAVABOT_LOGIC_H

// Robot states
enum RobotMode {
    MANUAL,
    AUTO,
    EMERGENCY
};

struct RobotState {
    RobotMode mode;
    bool uvOn;
    bool relay2On;
    bool moving;
};

// Logic functions
RobotState startAutoMode();
RobotState triggerEmergencyStop();
RobotState resetEmergency();
RobotState finishUvCycle();

#endif
