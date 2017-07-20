#pragma once

#include "DCMotor.h"
#include "Encoder.h"

#include "Exception.h"

#include <ArduinoJson.h>

//a class for managing an axis (i.e. an encoder and a motor)
class Axis {
public:
	struct Config {
		Config();

		float stallTorque;

		int pulseCount;
		float pulseTorque; /* 0 = use stall torque. 1 = use full torque */
		int pulseDuration; // ms
		int pulseDurationPerStep; //ms extra per step
		int pulseWait; // ms between pulses
		int pulseSettle; // ms after last pulse before taking a measurement

		bool switchOutputPolarity; // false = ouput polarity matches input polarity
	};

	DCMotor motor;
	Encoder encoder;

	void setup(const DCMotor::Pins & motorPins
					, const Encoder::Pins & encoderPins
					, const Config & config = Config());

	Exception update();

	// This function tries to navigate
	Exception checkPolarity();
	float getOutputPolarity() const;

	void setDriveEnabled(bool);
	void navigateTo(Encoder::Position);
	void walk(Encoder::PositionDelta deltaPosition);
	void pulseAxis(uint16_t pulseCount, float torque, uint16_t durationMillis, uint16_t delayMillis);
	void reportStatus(JsonObject &) const;
protected:
	Encoder::Position targetPosition = 0;
	Encoder::Position nearDistance = 10;

	Config config;

	float velocity = 0.0f;
	unsigned long lastFrameTime = 0;
	Encoder::Position lastFramePosition = 0;

	bool driveEnabled = false;
};
