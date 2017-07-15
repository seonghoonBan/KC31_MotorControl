#include <Arduino.h>

#include "Axis.h"
#include "Commands.h"

#define AXIS_COUNT 2

Axis axis[AXIS_COUNT];

void setup() {
    setupCommands();
    axis[0].setup(DCMotor::Pins {11, 9, 10}, Encoder::Pins {2, 3}, 0.3f);
    //axis[1].setup(DCMotor::Pins {5, 6, 7}, Encoder::Pins {0, 1}, 0.3f);
}

void loop() {
    cmdMessenger.feedinSerialData();
    {
        for(uint8_t axisIndex = 0; axisIndex < AXIS_COUNT; axisIndex++) {
            auto exception = axis[axisIndex].update();
            if(exception.errorNumber != Error::NoError) {
                sendException(exception);
            }
        }
    }
}

void printStatus() {
    for(uint8_t axisIndex = 0; axisIndex < AXIS_COUNT; axisIndex++) {
        Serial.print("Axis [");
        Serial.print((int) axisIndex);
        Serial.println("] : ");

        axis[axisIndex].printStatus();
        Serial.println();
    }
}

void enableAxes() {
    for(uint8_t axisIndex = 0; axisIndex < AXIS_COUNT; axisIndex++) {
        axis[axisIndex].enable();
    }
}

void disableAxes() {
    for(uint8_t axisIndex = 0; axisIndex < AXIS_COUNT; axisIndex++) {
        axis[axisIndex].disable();
    }
}

void axisGoto(uint8_t axisIndex, long long position) {
    axis[axisIndex].navigateTo(position);
}

void axisWalk(uint8_t axisIndex, long long deltaPosition) {
    axis[axisIndex].walk(deltaPosition);
}

void axisPulse(uint8_t axisIndex, uint16_t pulseCount, float torque, uint16_t durationMillis, uint16_t delayMillis) {
    axis[axisIndex].pulseAxis(pulseCount, torque, durationMillis, delayMillis);
}

void axisTare(uint8_t axisIndex, long long positionMark) {
    axis[axisIndex].encoder.tarePosition(positionMark);

    //also we'll cancel any naviations
    axis[axisIndex].navigateTo(positionMark);
}

long long getAxisPosition(uint8_t axisIndex) {
    return axis[axisIndex].encoder.getPosition();
}
