/**
 * \file XBee AT SerialBreak
 * \author Alex Livings
 *
 * Uses a Serial Break to force the connected XBee device to AT Command Mode
 * with a 9600 baud rate. Changes the XBee device's baud rate setting to match
 * the baud rate setting of the ClearCore's software XBee object.
 *
 * For a more complete list of available AT commands, search the internet.
 */

#include "ClearCore.h"
#include "SysTiming.h"

#define TIMEOUT 10000
#define START_WAIT 3000

#define OK_MSG "OK\0"
#define ERR_MSG "ERROR\0"
#define TIMEOUT_MSG "\0"

// Track Time in AT Mode
uint32_t msLastCmd = 0;

// command send arguments
char sendStr[10];
char rspStr[10];
uint8_t rspStrSize = 10;

void setup() {
    // initialize serial
    Serial.begin(115200);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    // Set the software speed configuration
    ClearCore::XBee.Speed(19200);
    // Set the XBee device speed
    DeviceSpeed();
}

void loop() {}

/**
 * Puts the XBee into default AT Command Mode using a serial break,
 * which defaults the XBee's baud rate to 9600.
 *
 * Sets the baud rate for the XBee device to match the baud rate
 * for the ClearCore's XBee manager.
 */
void DeviceSpeed() {
    uint32_t speed = ClearCore::XBee.Speed();
    ClearCore::XBee.Speed(9600);

    // Force AT Command Mode with default 9600 baud rate
    AtModeDefault();
    if (strcmp(rspStr, OK_MSG)) {
        exitFail("Failed to establish AT Command Mode.");
    }

    // Ask if AT Command Mode is Active
    atCommandSend("AT", rspStr, rspStrSize);
    if (strcmp(rspStr, OK_MSG)) {
        exitFail("Failed to verify AT Command Mode.");
    }

    // Set the XBee device's baud rate
    char baudRate = '0';
    switch (speed) {
        case 2400:
            baudRate = '1';
            break;
        case 4800:
            baudRate = '2';
            break;
        case 9600:
            baudRate = '3';
            break;
        case 19200:
            baudRate = '4';
            break;
        case 38400:
            baudRate = '5';
            break;
        case 57600:
            baudRate = '6';
            break;
        case 115200:
            baudRate = '7';
            break;
        case 230400:
            baudRate = '8';
            break;
        case 460800:
            baudRate = '9';
            break;
        case 921600:
            baudRate = 'A';
            break;
        default:
            exitFail("Specified baud rate is invalid.");
            break;
    }

    strncpy(sendStr, "ATBD ", 5);
    sendStr[5] = baudRate;
    sendStr[6] = '\0';
    // Set the XBee device's baud rate
    atCommandSend(sendStr, rspStr, rspStrSize);
    if (strcmp(rspStr, OK_MSG)) {
        exitFail("Failed to set the specified baud rate.");
    }

    // Save the configuration to XBee firmware
    atCommandSend("ATWR", rspStr, rspStrSize);
    if (strcmp(rspStr, OK_MSG)) {
        exitFail("Failed to write to firmware.");
    }

    // Exit AT Command Mode
    atCommandSend("ATCN", rspStr, rspStrSize);
    if (strcmp(rspStr, OK_MSG)) {
        exitFail("Failed to force exit AT Command Mode.");
    }

    ClearCore::XBee.Speed(speed);
}

/**
 * Holds a serial break for 6 seconds to enter AT Command Mode.
 */
void AtModeDefault() {
    Serial.println("Forcing AT Mode with a Serial Break...");
    ClearCore::XBee.SerialBreak(true);  // Start Serial Break
    delay(6000);                        // wait for 6s
    ClearCore::XBee.SerialBreak(false); // End Serial Break
    msLastCmd = Milliseconds();
    atRspReceive(rspStr, rspStrSize);
    delay(1);
    msLastCmd = Milliseconds();
}

/**
 * High-level helper function to send a command.
 */
void atCommandSend(char *cmd, char *rsp, uint8_t rspLen) {
    // check if 10 seconds since last command
    if ((Milliseconds() - msLastCmd) > TIMEOUT) {
        return; // no longer in AT mode
    }
    ClearCore::XBee.Send(cmd); // send the command
    Serial.println(cmd);
    ClearCore::XBee.Send(
        "\r"); // any command other than "+++" requires carriage return
    msLastCmd = Milliseconds(); // reset the timer
    atRspReceive(rsp, rspLen);
}

/**
 * Wait for a complete response (always ends in carriage return)
 */
bool atRspReceive(char *rsp, uint8_t rspLen) {
    uint8_t rspIndex = 0;
    char readChar;
    rsp[rspIndex] = '\0';
    while ((Milliseconds() - msLastCmd) < TIMEOUT) {
        if (ClearCore::XBee.AvailableForRead()) {
            if (rspIndex < rspLen - 1) {
                readChar = ClearCore::XBee.CharGet();
                if (readChar == 0x0D) {
                    Serial.println(rsp);
                    return true;
                }
                rsp[rspIndex++] = readChar;
                rsp[rspIndex] = '\0';
            }
        }
    }
    Serial.println("Timed out waiting for a response.");
    return false;
}

/**
 *  Indicates a failed test.
 */
void exitFail(String message) {
    Serial.println(message);
    delay(600);
}