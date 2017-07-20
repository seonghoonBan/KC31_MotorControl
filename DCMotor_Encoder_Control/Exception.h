#pragma once

typedef unsigned long ErrorType;
enum Error : ErrorType {
    NoError = 0,

    GeneralTimeout,
    
    UnknownCommand,
    PulseOverrun, // whilst navigating in the near zone, we took too many pulses to adjust our position

    UnknownError
};

struct Exception {
    Exception() {

    }
    Exception(Error errorNumber) {
        this->errorNumber = errorNumber;
    }
    Exception(Error errorNumber, int32_t argument)
    : Exception(errorNumber) {
        this->arguments[0] = argument;
        this->argumentCount = 1;
    }

    operator bool() const {
        return this->errorNumber != Error::NoError;
    }

    Error errorNumber = Error::NoError;
    int32_t arguments[8];
    uint8_t argumentCount = 0;
    //consider having a variable number of constructor arguments
};
