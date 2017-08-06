#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class DCMotor
{
  public:
	struct Pins
	{
		int enable;
		int A;
		int B;
	};

	void setup(const Pins &);
	void setTorque(float);
	void reportStatus(JsonObject &) const;

  protected:
	Pins pins;
	float torque = 0.0f;
};
