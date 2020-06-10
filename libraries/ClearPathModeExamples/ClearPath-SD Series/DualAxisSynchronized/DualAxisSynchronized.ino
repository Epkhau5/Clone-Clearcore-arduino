/*
 * Title: DualAxisSynchronized
 *
 * Objective:
 *    This example demonstrates control of two ClearPath-SD motors synchronously
 *    in Step and Direction mode. Use this example when two motors must follow
 *    identical commands (e.g. a mechanically connected dual-axis or gantry).
 *
 * Description:
 *    This example enables two motors then commands a repeating series of
 *    synchronized moves. Move status is printed to the USB serial port. This
 *    example commands a max travel of 25600 pulses.
 *
 * Requirements:
 * 1. Two ClearPath motors must be connected, one to Connector M-0 the other to
 *    Connector M-1.
 * 2. The connected ClearPath motors must be configured through the MSP software
 *    for Step and Direction mode (In MSP select Mode>>Step and Direction).
 * 3. The ClearPath motors must be set to use the HLFB mode "ASG-Position"
 *    through the MSP software (select Advanced>>High Level Feedback [Mode]...
 *    then choose "All Systems Go (ASG) - Position" from the dropdown and hit
 *    the OK button).
 * 4. If the two motors must spin in opposite directions (i.e. they are mounted
 *    facing different directions), check the "Reverse Direction" checkbox of
 *    one motor in MSP.
 *
 * ** Note: Homing is optional, and not required in this operational mode or in
 *    this example. This example makes its first move in the positive direction,
 *    assuming any homing move occurs in the negative direction.
 *
 * ** Note: Set the Input Resolution in MSP the same as your motor's Positioning
 *    Resolution spec if you'd like the pulses sent by ClearCore to command a
 *    move of the same number of Encoder Counts, a 1:1 ratio.
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

// Specify which motors to move.
// Options are: ConnectorM0, ConnectorM1, ConnectorM2, or ConnectorM3.
#define motor0 ConnectorM0
#define motor1 ConnectorM1

// Select the baud rate to match the target serial device
#define baudRate 9600

// Define the velocity and acceleration limits to be used for each move
int velocityLimit = 10000; // pulses per sec
int accelerationLimit = 100000; // pulses per sec^2

// Declares our user-defined helper function, which is used to move both motors
// synchronously. The definition/implementation of this function is at the
// bottom of the example.
void SynchronizedMove(int distance);

void setup() {
    // Put your setup code here, it will only run once:

    // Sets the input clocking rate. This normal rate is ideal for ClearPath
    // step and direction applications
    MotorMgr.MotorInputClocking(MotorManager::CLOCK_RATE_NORMAL);

    // Sets all motor connectors into step and direction mode.
    MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL,
                          Connector::CPM_MODE_STEP_AND_DIR);

    // Sets the maximum velocity for each move
    motor0.VelMax(velocityLimit);
    motor1.VelMax(velocityLimit);

    // Sets the maximum acceleration for each move
    motor0.AccelMax(accelerationLimit);
    motor1.AccelMax(accelerationLimit);

    // Sets up serial communication and waits up to 5 seconds for a port to open
    // Serial communication is not required for this example to run
    Serial.begin(baudRate);
    uint32_t timeout = 5000;
    uint32_t startTime = millis();
    while (!Serial && millis() - startTime < timeout) {
        continue;
    }

    // Enables the motors; homing will begin automatically if enabled in MSP
    motor0.EnableRequest(true);
    Serial.println("Motor 0 Enabled");
    motor1.EnableRequest(true);
    Serial.println("Motor 1 Enabled");

    // Waits for both motors to finish enabling
    uint32_t lastStatusTime = millis();
    while (motor0.HlfbState() != MotorDriver::HLFB_ASSERTED ||
            motor1.HlfbState() != MotorDriver::HLFB_ASSERTED) {
        // Periodically prints out why the application is waiting
        if (millis() - lastStatusTime > 100) {
            Serial.println("Waiting for HLFB to assert on both motors");
            lastStatusTime = millis();
        }
    }
    Serial.println("Motors Ready");
}

void loop() {
    // Move 6400 counts (positive direction) then wait 2000ms.
    SynchronizedMove(6400);
    delay(2000);
    // Move 19200 counts farther positive, then wait 2000ms.
    SynchronizedMove(19200);
    delay(2000);
    // Move back 12800 counts (negative direction), then wait 2000ms.
    SynchronizedMove(-12800);
    delay(2000);
    // Move back 6400 counts (negative direction), then wait 2000ms.
    SynchronizedMove(-6400);
    delay(2000);
    // Move back to the start (negative 6400 pulses), then wait 2000ms.
    SynchronizedMove(-6400);
    delay(2000);
}

/*------------------------------------------------------------------------------
 * SynchronizedMove
 *
 *    Moves two motors an incremental distance synchronously.
 *    Prints the move status to the USB serial port
 *    Returns when HLFB asserts (indicating the motor has reached the commanded
 *    position)
 *
 * Parameters:
 *    int distance  - The distance, in counts, to move
 *
 * Returns: None
 */
void SynchronizedMove(int distance) {
    Serial.print("Moving distance: ");
    Serial.println(distance);

    // Move both motors the same distance
    motor0.Move(distance);
    motor1.Move(distance);

    // Wait until both motors complete their moves
    uint32_t lastStatusTime = millis();
    while (!motor0.StepsComplete() || motor0.HlfbState() != MotorDriver::HLFB_ASSERTED ||
            !motor1.StepsComplete() || motor1.HlfbState() != MotorDriver::HLFB_ASSERTED) {
        // Periodically print out why the application is waiting
        if (millis() - lastStatusTime > 100) {
            Serial.println("Waiting for HLFB to assert on both motors");
            lastStatusTime = millis();
        }
    }

    Serial.println("Move Done");
}
//------------------------------------------------------------------------------