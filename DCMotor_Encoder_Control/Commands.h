#pragma once

#include <CmdMessenger.h>
#include "Exception.h"

char fieldSeparator = ',';
char commandSeparator = ';';
char escapeCharacter = '/';

enum Commands : byte
{
	noCommand = 0,
	
	cmdHelp, // 1
	cmdError, // 2
	cmdPrintStatus, // 3
	cmdEnableDrive, // 4
	cmdDisableDrive, // 5
	cmdGetPosition, // 6
	cmdGotoPosition, // 7
	cmdWalk, // 8
	cmdPulse, // 9
	cmdTare, // 10

	responseAcknowledge,
	responseText,
	reponseGetPosition
};

CmdMessenger cmdMessenger = CmdMessenger(Serial, fieldSeparator, commandSeparator, escapeCharacter);

void attachCommandCallbacks();
void printStatus();
void enableDrive();
void disableDrive();
void gotoPosition(Encoder::Position position);
void walk(Encoder::PositionDelta deltaPosition);
void pulse(uint16_t pulseCount, float torque, uint16_t durationMillis, uint16_t delayMillis);
void tare(Encoder::Position positionMark);

Encoder::Position getPosition();

void acknowledgeCommand(byte commandIndex)
{
	cmdMessenger.sendBinCmd<byte>(responseAcknowledge, commandIndex);
}

void onCmdHelp()
{
	acknowledgeCommand(cmdHelp);

	auto helpString = "Available commands:"
	"0   - Acknowledge"
	"1   - Print help"
	"2   - Error"
	"3   - Print status"
	"4   - Enable drive"
	"5   - Disable drive"
	"6   - Get axis position"
	"7   - Goto position <int position>"
	"8   - Walk <int delta>"
	"9   - Pulse <int count, float torque, int durationMillis, int delayMillis>"
	"10  - Tare <int positionMark>";

	cmdMessenger.sendCmd(responseText, helpString);
}

void onCmdPrintStatus()
{
	acknowledgeCommand(cmdPrintStatus);

	printStatus();
}

void onCmdDisableDrive()
{
	acknowledgeCommand(cmdDisableDrive);
	disableDrive();
}

void onCmdEnableDrive()
{
	acknowledgeCommand(cmdEnableDrive);
	enableDrive();
}

void sendPositionResponse()
{
	cmdMessenger.sendBinCmd(reponseGetPosition, getPosition());
}

void onCmdGetPosition()
{
	acknowledgeCommand(cmdGetPosition);

	sendPositionResponse();
}

void onCmdGotoPosition()
{
	acknowledgeCommand(cmdGotoPosition);
	auto position = cmdMessenger.readBinArg<Encoder::Position>();
	gotoPosition(position);
}

void onCmdWalk()
{
	acknowledgeCommand(cmdWalk);
	walk(cmdMessenger.readBinArg<Encoder::PositionDelta>());
}

void onCmdPulse()
{
	acknowledgeCommand(cmdPulse);

	auto pulseCount = (uint16_t) cmdMessenger.readBinArg<unsigned int>();
	auto torque = cmdMessenger.readBinArg<float>();
	auto durationMillis = cmdMessenger.readBinArg<unsigned int>();
	auto delayMillis = cmdMessenger.readBinArg<unsigned int>();

	pulse(pulseCount, torque, durationMillis, delayMillis);

	sendPositionResponse();
}

void onCmdTare()
{
	acknowledgeCommand(cmdTare);
	auto positionMark = cmdMessenger.readBinArg<Encoder::Position>();
	tare(positionMark);
}

void onCmdUnknown()
{
	cmdMessenger.sendBinCmd<ErrorType>(cmdError, Error::UnknownCommand);
	onCmdHelp();
}

void setupCommands()
{
	Serial.begin(115200);
	attachCommandCallbacks();
	cmdMessenger.sendCmd(responseText, "Kimchi and Chips' CLMP v 1.0");
}

void attachCommandCallbacks()
{
	cmdMessenger.attach(onCmdUnknown);
	cmdMessenger.attach(cmdHelp, onCmdHelp);
	cmdMessenger.attach(cmdPrintStatus, onCmdPrintStatus);
	cmdMessenger.attach(cmdEnableDrive, onCmdEnableDrive);
	cmdMessenger.attach(cmdDisableDrive, onCmdDisableDrive);
	cmdMessenger.attach(cmdGetPosition, onCmdGetPosition);
	cmdMessenger.attach(cmdGotoPosition, onCmdGotoPosition);
	cmdMessenger.attach(cmdWalk, onCmdWalk);
	cmdMessenger.attach(cmdPulse, onCmdPulse);
	cmdMessenger.attach(cmdTare, onCmdTare);
}

void sendException(const Exception &exception)
{
	cmdMessenger.sendCmdStart(cmdError);
	cmdMessenger.sendCmdBinArg<int>(exception.errorNumber);
	for (uint8_t i = 0; i < exception.argumentCount; i++)
	{
		cmdMessenger.sendCmdBinArg<int>(exception.arguments[i]);
	}
	cmdMessenger.sendCmdEnd();
}
