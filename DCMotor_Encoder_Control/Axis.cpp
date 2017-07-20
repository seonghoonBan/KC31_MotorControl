#include "Axis.h"

//utility function to determine if a number is positive, negative or zero
template<typename T>
T sign(T value) {
	if (value > 0) {
		return 1.0f;
	} else if (value < 0) {
		return -1.0;
	} else {
		return 0.0;
	}
}

//----------
void Axis::setup(const DCMotor::Pins & motorPins
				, const Encoder::Pins & encoderPins
				, const Config & config) {
	this->motor.setup(motorPins);
	this->encoder.setup(encoderPins, Encoder::InterruptType::Pin);
	this->config = config;

	this->setEnabled(false);
}

//----------
Exception Axis::update() {
	this->encoder.update();

	if(this->enabled) {
		float deltaT;
		{
			auto now = micros();
			auto deltaTMicros = lastFrameTime - now;
			lastFrameTime = now;

			deltaT = float(deltaTMicros) / 1000000.0f;
		}

		auto position = this->encoder.getPosition();
		this->velocity = float(position - this->lastFramePosition) / deltaT;
		this->lastFramePosition = position;

		auto deltaPosition = this->targetPosition - position;

		float torque;
		if(abs(deltaPosition) <= this->nearDistance) {
			// NEAR ZONE

			//calculate pulse power
			float pulsePower = 1.0f - this->config.stallTorque;
			pulsePower *= PULSE_TORQUE;
			pulsePower += this->config.stallTorque;

			//walk the last few steps when we're close
			for(uint8_t i=0; i<PULSE_COUNT; i++) {
					auto pulseCount = abs(deltaPosition) > 4 ? abs(deltaPosition) / 2 : 1;

					this->pulseAxis(1
						, pulsePower * sign(deltaPosition)
						, PULSE_DURATION + PULSE_DURATION_PERSTEP * abs(deltaPosition)
						, PULSE_WAIT);

					//pause a little while to settle after last pulse
					delay(PULSE_SETTLE);

					this->encoder.update();
					position = this->encoder.getPosition();
					deltaPosition = this->targetPosition - position;

					if(deltaPosition == 0) {
						break;
					}
			}
			if(deltaPosition != 0) {
					//ERROR!
					this->motor.setTorque(0.0f);
					return Exception(Error::PulseOverrun, deltaPosition);
			}

			//turn off any residual torque commands (e.g. if there were zero pulses)
			this->motor.setTorque(0.0f);

		} else {
			// FAR ZONE

			//accelerate towards targetPosition
			torque = float(deltaPosition) / 300.0f;
			if(abs(torque) < this->config.stallTorque) {
				torque += this->config.stallTorque * sign(torque);
			}
			this->motor.setTorque(torque);
		}
	} else {
		this->motor.setTorque(0.0f);
	}

	return Error::NoError;
}

//----------
void Axis::setEnabled(bool enabled) {
	this->enabled = enabled;
}

//----------
void Axis::navigateTo(Encoder::Position position) {
	this->targetPosition = position;
}

//----------
void Axis::walk(Encoder::PositionDelta deltaPosition) {
	this->targetPosition = this->encoder.getPosition() + deltaPosition;
}

//---------
void Axis::pulseAxis(uint16_t pulseCount, float torque, uint16_t durationMillis, uint16_t delayMillis) {
	for(uint16_t pulseIndex = 0; pulseIndex < pulseCount; ++pulseIndex) {
		this->motor.setTorque(torque);
		delay(durationMillis);
		this->motor.setTorque(0.0f);

		if(pulseIndex != pulseCount - 1) {
			delay(delayMillis);
		}
	}
}

//---------
void Axis::printStatus() const {
	Serial.print("Enabled : " );
	Serial.println(this->enabled ? "True" : "False");

	Serial.print("Target position : " );
	Serial.println((int) this->targetPosition);

	Serial.print("Velocity : " );
	Serial.println(this->velocity);

	Serial.println("Encoder : ");
	this->encoder.printStatus();
	Serial.println("Motor : ");
	this->motor.printStatus();
}