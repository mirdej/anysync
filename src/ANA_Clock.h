#include "Arduino.h"
#include <ESPLogger.h>

#pragma once

extern ESPLogger logger;

class ANA_Clock {
    public:
        ANA_Clock();
        begin();
       uint32_t millis() {return _millis_corrected;}


    private:
        uint32_t _millis_corrected;


};
