#pragma once

enum Error : uint32_t {
    NoError = 0,
    
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

    Error errorNumber = Error::NoError;
    int32_t arguments[8];
    uint8_t argumentCount = 0;
    //consider having a variable number of constructor arguments
};
