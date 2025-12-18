#include "lavabot_logic.h"

RobotState startAutoMode() {
    RobotState state;
    state.mode = AUTO;
    state.uvOn = true;
    state.relay2On = false;
    state.moving = false; // intentional defect
    return state;
}

RobotState triggerEmergencyStop() {
    return {
        EMERGENCY,
        false,
        false,
        false
    };
}

RobotState resetEmergency() {
    return {
        MANUAL,
        false,
        false,
        false
    };
}

RobotState finishUvCycle() {
    return {
        AUTO,
        false,
        true,
        false
    };
}
