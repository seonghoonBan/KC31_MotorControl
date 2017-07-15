#include "Encoder.h"
#include "Arduino.h"

//----------
Encoder::Encoder() {
    auto next = Encoder::getFirst();
    if(!next) {
      Encoder::getFirst() = this;
    } else {
      Encoder * last;
      do {
        last = next;
        next = next->getNext();
      } while(next);
      last->getNext() = this;
    }
}

//----------
void Encoder::setup(const Pins & pins, InterruptType interruptType) {
  this->pins = pins;

  pinMode(this->pins.A, INPUT_PULLUP);
  pinMode(this->pins.B, INPUT_PULLUP);

  this->state = Encoder::calculateState(digitalRead(this->pins.A), digitalRead(this->pins.B));

  switch(interruptType) {
      case InterruptType::Pin:
      {
          attachInterrupt(digitalPinToInterrupt(this->pins.A), encoderPinChanged, CHANGE);
          attachInterrupt(digitalPinToInterrupt(this->pins.B), encoderPinChanged, CHANGE);
          break;
      }
      case InterruptType::Timer:
      {
          initialiseTimer();
          break;
      }
      default:
          break;
  }
}

//----------
void Encoder::update() {
    this->processStateBuffer();
}

//----------
long long Encoder::getPosition() {
    this->processStateBuffer();
    return this->position;
}

//----------
void Encoder::tarePosition(long long positionMark) {
    this->position = positionMark;
}

//----------
unsigned long Encoder::getErrorCount() const {
  return this->errorCount;
}

//----------
void Encoder::printStatus() {
  Serial.print("Position : ");
  Serial.println((int) this->getPosition());

  Serial.print("Error count : ");
  Serial.println(this->getErrorCount());
}

//----------
void Encoder::pinChangeCallback() {
  auto A = digitalRead(this->pins.A);
  auto B = digitalRead(this->pins.B);
  auto state = Encoder::calculateState(A, B);

  this->writeStateBuffer(state);
}

//----------
void Encoder::timerCallback() {
  auto A = digitalRead(this->pins.A);
  auto B = digitalRead(this->pins.B);
  auto state = Encoder::calculateState(A, B);

  if(state != this->lastWroteState) {
      this->writeStateBuffer(state);
      this->lastWroteState = state;
  }
}

//----------
int8_t Encoder::calculateState(bool A, bool B) {
 return char(A^B)
       | char(B) << 1;
}

//----------
void Encoder::writeStateBuffer(int8_t state) {
#ifdef PRINT_READS_AND_WRITES
    Serial.print('W');
    Serial.print((int) state);
    Serial.print('(');
    Serial.print((int) this->stateCacheRingBuffer.writePosition);
    Serial.print(')');
#endif

    this->stateCacheRingBuffer.buffer[this->stateCacheRingBuffer.writePosition] = state;
    ++this->stateCacheRingBuffer.writePosition;
    stateCacheRingBuffer.writePosition %= StateCacheRingBuffer_SIZE;
}

//----------
void Encoder::processStateBuffer() {
    auto & stateCacheRingBufferLocal = this->stateCacheRingBuffer;

    while (stateCacheRingBufferLocal.readPosition !=  stateCacheRingBufferLocal.writePosition) {
        auto state = stateCacheRingBufferLocal.buffer[stateCacheRingBufferLocal.readPosition];

#ifdef PRINT_READS_AND_WRITES
        Serial.print('R');
        Serial.print((int) state);
        Serial.print('(');
        Serial.print((int) stateCacheRingBufferLocal.readPosition);
        Serial.print(')');
#endif

        if(state == this->state) {
            Serial.println('E');
        }

        this->processState(state);
        ++stateCacheRingBufferLocal.readPosition;
        stateCacheRingBufferLocal.readPosition %= StateCacheRingBuffer_SIZE;
    }

    //for next read, we'll start where we will end
    this->stateCacheRingBuffer.readPosition = stateCacheRingBufferLocal.readPosition;
}

//----------
void Encoder::processState(int8_t state) {
    auto delta = state - this->state;
    auto absDelta = abs(delta);

    switch(absDelta) {
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
        Serial.print((int) this->state);
        Serial.print(" -> ");
        Serial.println((int) state);
    #endif

        break;
    }

    this->state = state;
}

//----------
void encoderPinChanged() {
  auto encoder = Encoder::getFirst();
  while(encoder) {
    encoder->pinChangeCallback();
    encoder = encoder->getNext();
  }
}

//----------
void timerCallback() {
    auto encoder = Encoder::getFirst();
    while(encoder) {
      encoder->timerCallback();
      encoder = encoder->getNext();
    }
}

//----------
void initialiseTimer() {
    static bool timerInitialised = false;
    if(!timerInitialised) {
        FlexiTimer2::set(1, 1.0/10000, timerCallback);
        FlexiTimer2::start();
    }
    timerInitialised = true;
}
