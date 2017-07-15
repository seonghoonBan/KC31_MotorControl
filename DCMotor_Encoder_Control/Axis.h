#pragma once
#include "DCMotor.h"
#include "Encoder.h"

#include "Commands.h"
#include "Exception.h"

#define MAX_PULSE_NAVIGATIONS_IN_NEAR_ZONE 4

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

//a class for managing an axis (i.e. an encoder and a motor)
class Axis {
public:
  DCMotor motor;
  Encoder encoder;

  void setup(const DCMotor::Pins & motorPins
          , const Encoder::Pins & encoderPins
          , float stallTorque = 0.1f) {
    this->motor.setup(motorPins);
    this->encoder.setup(encoderPins, Encoder::InterruptType::Pin);
    this->stallTorque = stallTorque;
    this->disable();
  }

  Exception update() {
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

        //walk the last few steps when we're close
        for(uint8_t i=0; i<MAX_PULSE_NAVIGATIONS_IN_NEAR_ZONE; i++) {
            auto pulseCount = abs(deltaPosition) > 4 ? abs(deltaPosition) / 2 : 1;

            this->pulseAxis(pulseCount, this->stallTorque * sign(deltaPosition), 30, 500);

            //pause a little while to settle after last pulse
            delay(1000);

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
        if(abs(torque) < this->stallTorque) {
          torque += this->stallTorque * sign(torque);
        }
        this->motor.setTorque(torque);
      }
    } else {
      this->motor.setTorque(0.0f);
    }

    return Error::NoError;
  }

  void enable() {
    this->enabled = true;
  }

  void disable() {
    this->enabled = false;
  }

  void navigateTo(long long position) {
      this->targetPosition = position;
  }

  void walk(long long deltaPosition) {
    this->targetPosition = this->encoder.getPosition() + deltaPosition;
  }

  void pulseAxis(uint16_t pulseCount, float torque, uint16_t durationMillis, uint16_t delayMillis) {
    for(uint16_t pulseIndex = 0; pulseIndex < pulseCount; ++pulseIndex) {
      this->motor.setTorque(torque);
      delay(durationMillis);
      this->motor.setTorque(0.0f);

      if(pulseIndex != pulseCount - 1) {
         delay(delayMillis);
      }
    }
  }

  void printStatus() {
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
protected:
  long long targetPosition = 0;
  long long nearDistance = 10;

  float stallTorque = 0.1f;

  float velocity = 0.0f;
  long long lastFrameTime = 0;
  long long lastFramePosition = 0;

  bool enabled = false;
};
