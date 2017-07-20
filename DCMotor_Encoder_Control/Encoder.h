#pragma once
#include "FlexiTimer2.h"

void encoderPinChanged();
void timerCallback();
void initialiseTimer();

//#define PRINT_READS_AND_WRITES

class Encoder
{
  public:
	typedef long Position;
	typedef Position PositionDelta;
	
	enum InterruptType
	{
		Timer,
		Pin
	};

	struct Pins
	{
		int A;
		int B;
	};

	Encoder();

	void setup(const Pins &pins, InterruptType interruptType);
	void update();

	Position getPosition() const;
	void tarePosition(Position positionMark);
	unsigned long getErrorCount() const;

	void printStatus() const;

	void pinChangeCallback();
	void timerCallback();

	static Encoder *&getFirst()
	{
		static Encoder *first = nullptr;
		return first;
	}

	Encoder *&getNext()
	{
		return this->next;
	}

  protected:
	static int8_t calculateState(bool A, bool B);

	void writeStateBuffer(int8_t state);
	void processStateBuffer() const;
	void processState(int8_t state);

	Pins pins;

	mutable int8_t lastWroteState = -1; // for timer only
	mutable volatile int8_t state = -1; //-1 = uninitialised state
	Position position = 0;
	unsigned long errorCount = 0;

	Encoder *next = nullptr;

#define StateCacheRingBuffer_SIZE 64
	struct StateCacheRingBuffer
	{
		volatile int8_t buffer[StateCacheRingBuffer_SIZE];
		volatile uint8_t readPosition = 0;
		volatile uint8_t writePosition = 0;
	};

	mutable StateCacheRingBuffer stateCacheRingBuffer;
};
