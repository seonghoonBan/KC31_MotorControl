#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

//comment out this line if you want to support multiple encoders
#define ENCODER_ONLY_ONE

//uncomment this line if you want to print errors to serial
//#define ENCODER_PRINT_ERROR_STATES

class Encoder
{
  public:
	typedef long Position;
	typedef Position PositionDelta;
	
	typedef int8_t State;

	struct Pins
	{
		int A;
		int B;
	};

	Encoder();

	void setup(const Pins &pins);

	Position getPosition() const;
	
	void tarePosition(Position positionMark);
	unsigned long getErrorCount() const;
	void reportStatus(JsonObject &) const;
	void pinChangeCallback();

	static Encoder *&getFirst()
	{
		static Encoder *first = nullptr;
		return first;
	}

#ifdef ENCODER_ONLY_ONE
#else
	Encoder *&getNext()
	{
		return this->next;
	}
#endif

  protected:
	static State calculateState(bool A, bool B);
	void processState(State state);

	Pins pins;

	volatile State state = -1; //-1 = uninitialised state
	Position position = 0;
	unsigned long errorCount = 0;

#ifdef ENCODER_ONLY_ONE
#else
	Encoder *next = nullptr;
#endif
};
