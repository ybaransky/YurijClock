#pragma once
#include <RTCLib.h>

class RTC {
    RTC_DS3231* _rtcHard;
    RTC_Millis* _rtcSoft;
    
    public:
        void        init(void);
        DateTime    now(void);
        bool        attachSQWInterrupt(void (*callback)(void));
};