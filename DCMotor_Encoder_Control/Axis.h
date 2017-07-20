#pragma once
#include "DCMotor.h"
#include "Encoder.h"

#include "Commands.h"
#include "Exception.h"

#define PULSE_COUNT 16
#define PULSE_TORQUE 0 /* 0 = use stall torque. 1 = use full torque */
#define PULSE_DURATION 40
#define PULSE_DURATION_PERSTEP 5
#define PULSE_WAIT 100
#define PULSE_SETTLE 500

//a class for managing an axis (i.e. an encoder and a motor)
class Axis {
public:
	struct Config {
		float stallTorque;
		int pulseCount;
		float  pulseTorque;
		int pulseDuration;
		int pulseDurationPerStep;
		int pulseWait;
		int pulseSettle;
	};

	DCMotor motor;
	Encoder encoder;

	void setup(const DCMotor::Pins & motorPins
					, const Encoder::Pins & encoderPins
					, const Config & config = Config());

	Exception update();
	void setEnabled(bool);
	void navigateTo(Encoder::Position);
	void walk(Encoder::PositionDelta deltaPosition);
	void pulseAxis(uint16_t pulseCount, float torque, uint16_t durationMillis, uint16_t delayMillis);
	void printStatus() const;
protected:
	Encoder::Position targetPosition = 0;
	Encoder::Position nearDistance = 10;

	Config config;

	float velocity = 0.0f;
	unsigned long lastFrameTime = 0;
	Encoder::Position lastFramePosition = 0;

	bool enabled = false;
};
