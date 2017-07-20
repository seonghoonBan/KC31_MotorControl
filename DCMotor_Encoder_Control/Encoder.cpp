#include "Encoder.h"
#include "Arduino.h"

void encoderPinChanged();

//----------
Encoder::Encoder()
{
#ifdef ENCODER_ONLY_ONE
    Encoder::getFirst() = this;
#else
    auto next = Encoder::getFirst();
    if (!next)
    {
        Encoder::getFirst() = this;
    }
    else
    {
        Encoder *last;
        do
        {
            last = next;
            next = next->getNext();
        } while (next);
        last->getNext() = this;
    }
#endif
}

//----------
void Encoder::setup(const Pins &pins)
{
    this->pins = pins;

    pinMode(this->pins.A, INPUT_PULLUP);
    pinMode(this->pins.B, INPUT_PULLUP);

    this->state = Encoder::calculateState(digitalRead(this->pins.A), digitalRead(this->pins.B));

    attachInterrupt(digitalPinToInterrupt(this->pins.A), encoderPinChanged, CHANGE);
    attachInterrupt(digitalPinToInterrupt(this->pins.B), encoderPinChanged, CHANGE);
}    

//----------
Encoder::Position Encoder::getPosition() const
{
    return this->position;
}

//----------
void Encoder::tarePosition(Position positionMark)
{
    this->position = positionMark;
}

//----------
unsigned long Encoder::getErrorCount() const
{
    return this->errorCount;
}

//----------
void Encoder::reportStatus(JsonObject & json) const
{
    json["position"] = this->getPosition();
    json["errorCount"] = this->getErrorCount();
}

//----------
void Encoder::pinChangeCallback()
{
    auto A = digitalRead(this->pins.A);
    auto B = digitalRead(this->pins.B);
    auto state = Encoder::calculateState(A, B);

    this->processState(state);
}

//----------
Encoder::State Encoder::calculateState(bool A, bool B)
{
    return char(A ^ B) | char(B) << 1;
}

//----------
void Encoder::processState(State state)
{
    auto delta = state - this->state;
    auto absDelta = abs(delta);

    switch (absDelta)
    {
    case 3:
        delta = (delta < 0) ? 1 : -1;
    case 1:
        this->position += delta;
        break;
    case 0:
        break; //HACK - we shouldn't need this
    case 2:
    default:
        //nothing we can do here because we don't know which direction the turn is in
        this->errorCount++;

#ifdef ENCODER_PRINT_ERROR_STATES
        Serial.print("Error state : ");
        Serial.print((int)this->state);
        Serial.print(" -> ");
        Serial.println((int)state);
#endif
        break;
    }

    this->state = state;
}

//----------
void encoderPinChanged()
{
    auto encoder = Encoder::getFirst();
#ifdef ENCODER_ONLY_ONE
    encoder->pinChangeCallback();
#else
    while (encoder)
    {
        encoder->pinChangeCallback();
        encoder = encoder->getNext();
    }
#endif
}