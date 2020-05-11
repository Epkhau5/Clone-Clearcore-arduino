/*
 * Title: 16PositionsHomeToHardStop
 *
 * Objective:
 *    This example demonstrates control of the ClearPath-MCPV operational mode
 *    Move To Absolute Position, 16 Positions (Home to Hard Stop)
 *
 * Description:
 *    This example enables, homes, and then moves a ClearPath motor between
 *    preprogrammed absolute positions as defined in the MSP software. During
 *    operation, various move statuses are written to the USB serial port.
 *
 * Requirements:
 * 1. A ClearPath motor must be connected to Connector M-0.
 * 2. The connected ClearPath motor must be configured through the MSP software
 *    for Move To Absolute Position, 16 Positions (Home to Hard Stop) mode (In
 *    MSP select Mode>>Position>>Move to Absolute Position, then with "16
 *    Positions (Home to Hard Stop)" selected hit the OK button).
 * 3. The ClearPath motor must be set to use the HLFB mode "ASG-Position"
 *    through the MSP software (select Advanced>>High Level Feedback [Mode]...
 *    then choose "All Systems Go (ASG) - Position" from the dropdown and hit
 *    the OK button).
 * 4. The ClearPath must have defined Absolute Position Selections through
 *    the MSP software (On the main MSP window fill in the textboxes labeled
 *    1-16 found under "Position Selection Setup (cnts)").
 * 5. Homing must be configured in the MSP software for your mechanical
 *    system (e.g. homing direction, torque limit, etc.). To configure, click
 *    the "Setup..." button found under the "Homing" label on the MSP's main
 *    window.
 *
 * Links:
 * ** web link to doxygen (all Examples)
 * ** web link to ClearCore Manual (all Examples)  <<FUTURE links to Getting started webpage/ ClearCore videos>>
 * ** web link to ClearPath Operational mode video (Only ClearPath Examples)
 * ** web link to ClearPath manual (Only ClearPath Examples)
 *
 * Last Modified: 1/21/2020
 * Copyright (c) 2020 Teknic Inc. This work is free to use, copy and distribute under the terms of
 * the standard MIT permissive software license which can be found at https://opensource.org/licenses/MIT
 */

#include "ClearCore.h"

// The INPUT_A_B_FILTER must match the Input A, B filter setting in
// MSP (Advanced >> Input A, B Filtering...)
#define INPUT_A_B_FILTER 20

// Defines the motor's connector as ConnectorM0
#define motor ConnectorM0

// Declares our user-defined helper function, which is used to send move
// commands. The definition/implementation of this function is at the bottom of
// the sketch
bool MoveToPosition(int positionNum);

void setup() {
    // Put your setup code here, it will only run once:

    // Sets all motor connectors to the correct mode for Absolute Position mode
    MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL,
                          Connector::CPM_MODE_A_DIRECT_B_DIRECT);

    // Enforces the state of the motor's A and B inputs before enabling the motor
    motor.MotorInAState(false);
    motor.MotorInBState(false);

    // Sets up serial communication and wait up to 5 seconds for a port to open
    // Serial communication is not required for this example to run
    Serial.begin(9600);
    uint32_t timeout = 5000;
    uint32_t startTime = millis();
    while (!Serial && millis() - startTime < timeout) {
        continue;
    }

    // Enables the motor; homing will begin automatically
    motor.EnableRequest(true);
    Serial.println("Motor Enabled");

    // Waits for HLFB to assert (waits for homing to complete if applicable)
    Serial.println("Waiting for HLFB...");
    while (motor.HlfbState() != MotorDriver::HLFB_ASSERTED) {
        continue;
    }
    Serial.println("Motor Ready");
}


void loop() {
    // Put your main code here, it will run repeatedly:

    // Move to Position 1 defined in MSP
    MoveToPosition(1);    // See below for the detailed function definition
    // Wait 1000ms
    delay(1000);
    MoveToPosition(2);
    delay(1000);
    MoveToPosition(3);
    delay(1000);
    MoveToPosition(4);
    delay(1000);

    MoveToPosition(5);
    delay(1000);
    MoveToPosition(6);
    delay(1000);
    MoveToPosition(7);
    delay(1000);
    MoveToPosition(8);
    delay(1000);

    MoveToPosition(9);
    delay(1000);
    MoveToPosition(10);
    delay(1000);
    MoveToPosition(11);
    delay(1000);
    MoveToPosition(12);
    delay(1000);

    MoveToPosition(13);
    delay(1000);
    MoveToPosition(14);
    delay(1000);
    MoveToPosition(15);
    delay(1000);
    MoveToPosition(16);
    delay(1000);
}

/*------------------------------------------------------------------------------
 * MoveToPosition
 *
 *    Move to position number positionNum (defined in MSP)
 *    Prints the move status to the USB serial port
 *    Returns when HLFB asserts (indicating the motor has reached the commanded
 *    position)
 *
 * Parameters:
 *    int positionNum  - The position number to command (defined in MSP)
 *
 * Returns: True/False depending on whether the position was successfully
 * commanded.
 */
bool MoveToPosition(int positionNum) {
    Serial.print("Moving to position: ");
    Serial.println(positionNum);

    if (positionNum < 17 && positionNum > 0) {
        // Sends pulses on Input B based on positionNum
        for (int i = 0; i < positionNum; i++) {
            motor.MotorInBState(true);
            delay(INPUT_A_B_FILTER);
            motor.MotorInBState(false);
            delay(INPUT_A_B_FILTER);
        }

        // Triggers the command
        motor.MotorInAState(true);
        delay(INPUT_A_B_FILTER);
        motor.MotorInAState(false);
    }
    else {
        // If an invalid positionNum has been entered, returns a failure
        return false;
    }

    // Ensures this delay is at least 2ms longer than the Input A, B filter
    // setting in MSP
    delay(2 + INPUT_A_B_FILTER);

    // Waits for HLFB to assert (signaling the move has successfully completed)
    Serial.println("Moving.. Waiting for HLFB");
    while (motor.HlfbState() != MotorDriver::HLFB_ASSERTED) {
        continue;
    }

    Serial.println("Move Done");
    return true;
}
//------------------------------------------------------------------------------