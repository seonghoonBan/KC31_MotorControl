#include "DCMotor.h"

//----------
void DCMotor::setup(const Pins & pins)
{
	this->pins = pins;

	pinMode(this->pins.enable, OUTPUT);
	pinMode(this->pins.A, OUTPUT);
	pinMode(this->pins.B, OUTPUT);

	digitalWrite(this->pins.enable, LOW);
	digitalWrite(this->pins.A, LOW);
	digitalWrite(this->pins.B, LOW);

	this->setTorque(0.0f);
}

//----------
void DCMotor::setTorque(float torque)
{
	if (torque == 0.0f)
	{
		digitalWrite(this->pins.enable, LOW);
		digitalWrite(this->pins.A, LOW);
		digitalWrite(this->pins.B, LOW);
	}
	else
	{
		bool direction = torque > 0;

		if (abs(torque) > 1.0f)
		{
			torque /= abs(torque);
		}
		digitalWrite(this->pins.A, direction);
		digitalWrite(this->pins.B, !direction);

		analogWrite(this->pins.enable, (255.0f) * abs(torque));
	}

	this->torque = torque;
}

//----------
void DCMotor::reportStatus(JsonObject & json) const {
	json["torque"] = this->torque;
}