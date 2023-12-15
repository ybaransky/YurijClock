#pragma once
#include <RTClib.h>

class RTClock {
    RTC_DS3231* _rtcHard;
    RTC_Millis* _rtcSoft;
    
    public:
        void        init(void);
        bool        startTicking(void);
        void        adjust(const DateTime&);
        bool        noRealTime(void);
        bool        lostPower(void);

        DateTime    now(void);

    private:
        bool        _lostPower;
};
extern RTClock*         initRTClock(void);