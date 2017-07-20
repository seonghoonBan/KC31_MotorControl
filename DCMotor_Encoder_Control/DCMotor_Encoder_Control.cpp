#include <Arduino.h>

#include "Axis.h"
#include "Commands.h"

Axis axis;

void setup() {
    setupCommands();
    axis.setup(DCMotor::Pins {11, 10, 12}, Encoder::Pins {2, 3});
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

long getPosition() {
    return axis.encoder.getPosition();
}
