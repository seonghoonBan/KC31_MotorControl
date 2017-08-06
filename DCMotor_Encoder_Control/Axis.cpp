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
Axis::Config::Config() {
	this->stallTorque = 0.3f;
	this->pulseCount = 16;
	this->pulseTorque = 0;
	this->pulseDuration = 40;
	this->pulseDurationPerStep = 5;
	this->pulseWait = 100;
	this->pulseSettle = 500;

	this->switchOutputPolarity = false;
}

//----------
void Axis::setup(const DCMotor::Pins & motorPins
				, const Encoder::Pins & encoderPins
				, const Config & config) {
	this->motor.setup(motorPins);
	this->encoder.setup(encoderPins);
	this->config = config;

	this->setDriveEnabled(false);

	{
		this->checkPolarity(); // this will always walk off axis

		//walk back to home
		this->setDriveEnabled(true);
		while(!this->encoder.getPosition() == 0) {
			this->update();
		}
		this->setDriveEnabled(false);
	}
}

//----------
Exception Axis::update() {
	if(this->driveEnabled) {
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
			pulsePower *= this->config.pulseTorque;
			pulsePower += this->config.stallTorque;

			//walk the last few steps when we're close
			for(uint8_t i=0; i<this->config.pulseCount; i++) {
					//auto pulseCount = abs(deltaPosition) > 4 ? abs(deltaPosition) / 2 : 1;

					this->pulseAxis(1
						, pulsePower * sign(deltaPosition)
						, this->config.pulseDuration + this->config.pulseDurationPerStep * abs(deltaPosition)
						, this->config.pulseWait);

					//pause a little while to settle after last pulse
					delay(this->config.pulseSettle);

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
			this->motor.setTorque(torque * this->getOutputPolarity());
		}
	} else {
		this->motor.setTorque(0.0f);
	}

	return Exception(Error::NoError);
}

//----------
Exception Axis::checkPolarity() {
	auto wasDriveEnabled = this->driveEnabled;

	if(wasDriveEnabled) {
		this->setDriveEnabled(false);

		//wait until stopped
		bool success = false;
		for(int i=0; i<100; i++) {
			//check to see if any motion is detected
			auto startPosition = this->encoder.getPosition();
			delay(100);
			auto endPosition = this->encoder.getPosition();
			if(endPosition - startPosition == 0) {
				success = true;
				break;
			}
		}

		if(!success) {
			return Exception(Error::GeneralTimeout);
		}
	}

	this->config.switchOutputPolarity = false;

	auto startPosition = this->encoder.getPosition();
	this->pulseAxis(1, 1.0f, 100, 0);
	auto endPosition = this->encoder.getPosition();

	if(endPosition < startPosition) {
		this->config.switchOutputPolarity = true;
	}

	return Exception(Error::NoError);
}

//----------
float Axis::getOutputPolarity() const {
	return this->config.switchOutputPolarity ? -1.0f : 1.0f;
}

//----------
void Axis::setDriveEnabled(bool enabled) {
	this->driveEnabled = enabled;
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
		this->motor.setTorque(torque * this->getOutputPolarity());
		delay(durationMillis);
		this->motor.setTorque(0.0f);

		if(pulseIndex != pulseCount - 1) {
			delay(delayMillis);
		}
	}
}

//---------
void Axis::reportStatus(JsonObject & json) const {
	json["driveEnabled"] = this->driveEnabled;
	json["targetPosition"] = this->targetPosition;
	json["velocity"] = this->velocity;
	
	{
		auto & jsonEncoder = json.createNestedObject("encoder");
		this->encoder.reportStatus(jsonEncoder);
	}

	{
		auto & jsonMotor = json.createNestedObject("motor");
		this->motor.reportStatus(jsonMotor);
	}
}