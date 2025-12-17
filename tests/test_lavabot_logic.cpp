#include <cassert>
#include "lavabot_logic.h"

int main() {
    // Test AUTO mode start
    RobotState autoState = startAutoMode();
    assert(autoState.mode == AUTO);
    assert(autoState.uvOn == true);
    assert(autoState.relay2On == false);
    assert(autoState.moving == true);

    // Test EMERGENCY stop
    RobotState emergency = triggerEmergencyStop();
    assert(emergency.mode == EMERGENCY);
    assert(emergency.uvOn == false);
    assert(emergency.moving == false);

    // Test reset
    RobotState reset = resetEmergency();
    assert(reset.mode == MANUAL);

    // Test UV completion
    RobotState finished = finishUvCycle();
    assert(finished.uvOn == false);
    assert(finished.relay2On == true);

    return 0;
}
