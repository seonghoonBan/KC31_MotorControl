#pragma once

#include <CmdMessenger.h>
#include <ArduinoJson.h>
#include <avr/pgmspace.h>

#include "Exception.h"

char fieldSeparator = ',';
char commandSeparator = ';';
char escapeCharacter = '/';

enum Commands : byte
{
	noCommand = 0,
	
	cmdHelp, // 1
	cmdError, // 2
	cmdReportStatus, // 3
	cmdCheckPolarity, // 4
	cmdEnableDrive, // 5
	cmdDisableDrive, // 6
	cmdGetPosition, // 7
	cmdGotoPosition, // 8
	cmdWalk, // 9
	cmdPulse, // 10
	cmdTare, // 11
	cmdGetTemperatureAndHumidity, // 12

	responseAcknowledge,
	responseText,
	responseStatusReport,
	reponseGetPosition,
	responseTemperatureAndHumidity
};

CmdMessenger cmdMessenger = CmdMessenger(Serial, fieldSeparator, commandSeparator, escapeCharacter);
StaticJsonBuffer<200> jsonBuffer;

void attachCommandCallbacks();
void reportStatus(JsonObject &);
Exception checkPolarity();
void enableDrive();
void disableDrive();
void gotoPosition(Encoder::Position position);
void walk(Encoder::PositionDelta deltaPosition);
void pulse(uint16_t pulseCount, float torque, uint16_t durationMillis, uint16_t delayMillis);
void tare(Encoder::Position positionMark);
bool getTemperatureAndHumidity(float &, float &);

void sendException(const Exception &);

Encoder::Position getPosition();

void acknowledgeCommand(byte commandIndex)
{
	cmdMessenger.sendBinCmd<byte>(responseAcknowledge, commandIndex);
}

void sendPositionResponse()
{
	cmdMessenger.sendBinCmd(Commands::reponseGetPosition, getPosition());
}

void onCmdHelp()
{
	acknowledgeCommand(cmdHelp);

	const char helpString[] PROGMEM = "Available commands:"
	"0	- Acknowledge"
	"1	- Print help"
	"2	- Error"
	"3	- Report status"
	"4	- Check polarity"
	"5	- Enable drive"
	"6	- Disable drive"
	"7	- Get axis position"
	"8	- Goto position <int position>"
	"9	- Walk <int delta>"
	"10	- Pulse <int count, float torque, int durationMillis, int delayMillis>"
	"11	- Tare <int positionMark>"
	"12 - Get temperature and humidity";

	cmdMessenger.sendCmd(responseText, helpString);
}

void onCmdReportStatus()
{
	acknowledgeCommand(Commands::cmdReportStatus);

	JsonObject & root = jsonBuffer.createObject();

	reportStatus(root);

	char buffer[150];
	root.printTo(buffer, 150);

	cmdMessenger.sendCmd(Commands::responseStatusReport, buffer);
}

void onCmdCheckPolarity()
{
	acknowledgeCommand(Commands::cmdCheckPolarity);
	
	auto exception = checkPolarity();
	sendException(exception);
}

void onCmdDisableDrive()
{
	acknowledgeCommand(Commands::cmdDisableDrive);
	disableDrive();
}

void onCmdEnableDrive()
{
	acknowledgeCommand(Commands::cmdEnableDrive);
	enableDrive();
}

void onCmdGetPosition()
{
	acknowledgeCommand(Commands::cmdGetPosition);

	sendPositionResponse();
}

void onCmdGotoPosition()
{
	acknowledgeCommand(Commands::cmdGotoPosition);
	auto position = cmdMessenger.readBinArg<Encoder::Position>();
	gotoPosition(position);
}

void onCmdWalk()
{
	acknowledgeCommand(Commands::cmdWalk);
	walk(cmdMessenger.readBinArg<Encoder::PositionDelta>());
}

void onCmdPulse()
{
	acknowledgeCommand(Commands::cmdPulse);

	auto pulseCount = (uint16_t) cmdMessenger.readBinArg<unsigned int>();
	auto torque = cmdMessenger.readBinArg<float>();
	auto durationMillis = cmdMessenger.readBinArg<unsigned int>();
	auto delayMillis = cmdMessenger.readBinArg<unsigned int>();

	pulse(pulseCount, torque, durationMillis, delayMillis);

	sendPositionResponse();
}

void onCmdTare()
{
	acknowledgeCommand(Commands::cmdGetTemperatureAndHumidity);
	float temperature, humidity;
	if(getTemperatureAndHumidity(temperature, humidity)) {
		cmdMessenger.sendBinCmd(Commands::responseTemperatureAndHumidity, temperature, humidity);
	}
}

void onCmdGetTemperatureAndHumidity()
{
	acknowledgeCommand(Commands::cmdTare);
	auto positionMark = cmdMessenger.readBinArg<Encoder::Position>();
	tare(positionMark);
}

void onCmdUnknown()
{
	cmdMessenger.sendBinCmd<ErrorType>(Commands::cmdError, Error::UnknownCommand);
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
	cmdMessenger.attach(Commands::cmdHelp, onCmdHelp);
	cmdMessenger.attach(Commands::cmdReportStatus, onCmdReportStatus);
	cmdMessenger.attach(Commands::cmdCheckPolarity, onCmdCheckPolarity);
	cmdMessenger.attach(Commands::cmdEnableDrive, onCmdEnableDrive);
	cmdMessenger.attach(Commands::cmdDisableDrive, onCmdDisableDrive);
	cmdMessenger.attach(Commands::cmdGetPosition, onCmdGetPosition);
	cmdMessenger.attach(Commands::cmdGotoPosition, onCmdGotoPosition);
	cmdMessenger.attach(Commands::cmdWalk, onCmdWalk);
	cmdMessenger.attach(Commands::cmdPulse, onCmdPulse);
	cmdMessenger.attach(Commands::cmdTare, onCmdTare);
	cmdMessenger.attach(Commands::cmdGetTemperatureAndHumidity, onCmdGetTemperatureAndHumidity);
}

void sendException(const Exception & exception)
{
	if(exception.errorNumber != Error::NoError) {
		cmdMessenger.sendCmdStart(cmdError);
		cmdMessenger.sendCmdBinArg<int>(exception.errorNumber);
		for (uint8_t i = 0; i < exception.argumentCount; i++)
		{
			cmdMessenger.sendCmdBinArg<int>(exception.arguments[i]);
		}
		cmdMessenger.sendCmdEnd();
	}
}
