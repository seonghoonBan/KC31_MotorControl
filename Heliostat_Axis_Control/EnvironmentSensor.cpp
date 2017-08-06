#include "EnvironmentSensor.h"

//----------
bool EnvironmentSensor::getData(float & temperature, float & humidity) {
	if(this->sensors.read22(7) == DHTLIB_OK) {
        temperature = sensors.temperature;
        humidity = sensors.humidity;
        return true;
    }
    else {
        return false;
    }
}