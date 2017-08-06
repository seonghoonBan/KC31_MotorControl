#include <Arduino.h>

#include "Axis.h"
#include "EnvironmentSensor.h"

#include "Commands.h"
#include "Screen.h"

Axis axis;
EnvironmentSensor environmentSensor;
//Screen screen;

void setup() {
    setupCommands();
    axis.setup(DCMotor::Pins {11, 10, 12}, Encoder::Pins {2, 3});
    //screen.setup();
    Serial.begin(115200);
    Serial.println("we're now here");
}

void loop() {
    cmdMessenger.feedinSerialData();
    {
        auto exception = axis.update();
        if(exception.errorNumber != Error::NoError) {
            sendException(exception);
        }
    }
}

void reportStatus(JsonObject & json) {
    axis.reportStatus(json);
}

Exception checkPolarity() {
    return axis.checkPolarity();
}

void enableDrive() {
    axis.setDriveEnabled(true);
}

void disableDrive() {
    axis.setDriveEnabled(false);
}


void gotoPosition(Encoder::Position position) {
    axis.navigateTo(position);
}

void walk(Encoder::PositionDelta deltaPosition) {
    axis.walk(deltaPosition);
}

void pulse(uint16_t pulseCount, float torque, uint16_t durationMillis, uint16_t delayMillis) {
    axis.pulseAxis(pulseCount, torque, durationMillis, delayMillis);
}

void tare(Encoder::Position positionMark) {
    axis.encoder.tarePosition(positionMark);

    //also we'll cancel any naviations
    axis.navigateTo(positionMark);
}

bool getEnvironmentSensorData(float & temperature, float & humidity) {
    return environmentSensor.getData(temperature, humidity);
}

Encoder::Position getPosition() {
    return axis.encoder.getPosition();
}
