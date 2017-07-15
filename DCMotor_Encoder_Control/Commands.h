#pragma once

#include <CmdMessenger.h>
#include "Exception.h"

char fieldSeparator   = ',';
char commandSeparator = ';';

enum Commands : byte {
  cmdAcknowledge = 0,
  cmdHelp,
  cmdError,
  cmdPrintStatus,
  cmdEnableAxes,
  cmdDisableAxes,
  cmdGetAxisPosition,
  cmdAxisGoto,
  cmdAxisWalk,
  cmdAxisPulse,
  cmdAxisTare,


  reponseGetAxisPosition
};

CmdMessenger cmdMessenger = CmdMessenger(Serial, fieldSeparator, commandSeparator);

void attachCommandCallbacks();
void printStatus();
void enableAxes();
void disableAxes();
void axisGoto(uint8_t axisIndex, long long position);
void axisWalk(uint8_t axisIndex, long long deltaPosition);
void axisPulse(uint8_t axisIndex, uint16_t pulseCount, float torque, uint16_t durationMillis, uint16_t delayMillis);
void axisTare(uint8_t axisIndex, long long positionMark);
long long getAxisPosition(uint8_t axisIndex);

void acknowledgeCommand(byte commandIndex) {
  cmdMessenger.sendCmdStart(cmdAcknowledge);
  {
    cmdMessenger.sendCmdArg(commandIndex);
    cmdMessenger.sendCmdArg("Command recieved");
  }
  cmdMessenger.sendCmdEnd();
}

void onCmdHelp() {
  acknowledgeCommand(cmdHelp);

  Serial.println("Available commands:");
  Serial.println("0   - Acknowledge");
  Serial.println("1   - Print help");
  Serial.println("2   - Error");
  Serial.println("3   - Print status");
  Serial.println("4   - Enable axes");
  Serial.println("5   - Disable axes");
  Serial.println("6   - Get axis position <int axisIndex>");
  Serial.println("7   - Axis goto <int axisIndex, int position>");
  Serial.println("8   - Axis walk <int axisIndex, int delta>");
  Serial.println("9   - Pulse <int axisIndex, int count, float torque, int durationMillis, int delayMillis>");
  Serial.println("10  - Tare position <int axisIndex, int positionMark>");

  Serial.println();
}

void onCmdPrintStatus() {
  acknowledgeCommand(cmdPrintStatus);

  printStatus();
}

void onCmdDisableAxes() {
  acknowledgeCommand(cmdDisableAxes);
  disableAxes();
}

void onCmdEnableAxes() {
  acknowledgeCommand(cmdEnableAxes);
  enableAxes();
}

void sendPositionResponse(uint8_t axisIndex) {
    cmdMessenger.sendCmdStart(reponseGetAxisPosition);
    {
        cmdMessenger.sendCmdArg(axisIndex);
        cmdMessenger.sendCmdArg((int) getAxisPosition(axisIndex));
    }
    cmdMessenger.sendCmdEnd();
}

void onCmdGetAxisPosition() {
  acknowledgeCommand(cmdGetAxisPosition);

  auto axisIndex = (uint8_t) cmdMessenger.readInt16Arg();

  sendPositionResponse(axisIndex);
}

void onCmdAxisGoto() {
  acknowledgeCommand(cmdAxisGoto);

  auto axisIndex = (uint8_t) cmdMessenger.readInt16Arg();
  axisGoto(axisIndex, (long long) cmdMessenger.readInt32Arg());
}

void onCmdAxisWalk() {
  acknowledgeCommand(cmdAxisWalk);

  auto axisIndex = (uint8_t) cmdMessenger.readInt16Arg();
  axisWalk(axisIndex, (long long) cmdMessenger.readInt32Arg());
}

void onCmdAxisPulse() {
  acknowledgeCommand(cmdAxisPulse);

  auto axisIndex = (uint8_t) cmdMessenger.readInt16Arg();
  auto pulseCount = (uint16_t) cmdMessenger.readInt16Arg();
  auto torque = cmdMessenger.readFloatArg();
  auto durationMillis = cmdMessenger.readInt16Arg();
  auto delayMillis = cmdMessenger.readInt16Arg();

  axisPulse(axisIndex, pulseCount, torque, durationMillis, delayMillis);

  sendPositionResponse(axisIndex);
}

void onCmdAxisTare() {
    acknowledgeCommand(cmdAxisTare);
    auto axisIndex = (uint8_t) cmdMessenger.readInt16Arg();
    auto positionMark = cmdMessenger.readInt32Arg();
    axisTare(axisIndex, positionMark);
}

void onCmdUnknown() {
  cmdMessenger.sendCmd(cmdError, Error::UnknownCommand);
  onCmdHelp();
}

void setupCommands() {
  Serial.begin(115200);
  attachCommandCallbacks();
  cmdMessenger.printLfCr();
  cmdMessenger.sendCmd(cmdAcknowledge, "Kimchi and Chips' CLMP v 1.0");
}

void attachCommandCallbacks() {
  cmdMessenger.attach(onCmdUnknown);
  cmdMessenger.attach(cmdHelp, onCmdHelp);
  cmdMessenger.attach(cmdPrintStatus, onCmdPrintStatus);
  cmdMessenger.attach(cmdEnableAxes, onCmdEnableAxes);
  cmdMessenger.attach(cmdDisableAxes, onCmdDisableAxes);
  cmdMessenger.attach(cmdGetAxisPosition, onCmdGetAxisPosition);
  cmdMessenger.attach(cmdAxisGoto, onCmdAxisGoto);
  cmdMessenger.attach(cmdAxisWalk, onCmdAxisWalk);
  cmdMessenger.attach(cmdAxisPulse, onCmdAxisPulse);
  cmdMessenger.attach(cmdAxisTare, onCmdAxisTare);
}

void sendException(const Exception & exception) {
    cmdMessenger.sendCmdStart(cmdError);
    cmdMessenger.sendCmdArg(exception.errorNumber);
    for(uint8_t i=0; i<exception.argumentCount; i++) {
        cmdMessenger.sendCmdArg(exception.arguments[i]);
    }
    cmdMessenger.sendCmdEnd();
}
