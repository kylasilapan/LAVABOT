#include "lavabot_logic.h"

RobotState startAutoMode() {
    return {
        AUTO,
        true,   // UV ON
        false,  // Relay2 OFF
        true    // Moving
    };
}

RobotState triggerEmergencyStop() {
    return {
        EMERGENCY,
        false,  // UV OFF
        false,  // Relay2 OFF
        false   // Not moving
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
        false,  // UV OFF
        true,   // Relay2 ON
        false
    };
}
