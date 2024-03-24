#pragma once
#include <RTClib.h>

class RTClock {
    RTC_DS3231* _rtcHard;
    RTC_Millis* _rtcSoft;
    
    public:
        void        init(void);
        bool        start(void);
        void        adjust(const DateTime&);
        bool        hasDevice(void);
        bool        lostPower(void);

        DateTime    now(void);

    private:
        bool        _lostPower;
};
extern RTClock*       initRTClock(void);