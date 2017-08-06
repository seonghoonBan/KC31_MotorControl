#pragma once
#include <Arduino.h>
#include <dht.h>

class EnvironmentSensor {
public:
	bool getData(float & temperature, float & humidity);
protected:
	dht sensors;
};