#pragma once
#include <RTClib.h>

class RTClock {
    RTC_DS3231* _rtcHard;
    RTC_Millis* _rtcSoft;
    
    public:
        void        init(void);
        DateTime    now(void);
        void        adjust(const DateTime&);

        void        startTicking(void);
        void        tick();
};

class RTTimer {
    public:
        void        start(uint32_t, uint32_t interval);
        void        stop(void);
        bool        tick(uint32_t);
        uint32_t    count(void); 

    private:
        uint32_t _count;
        uint32_t _interval;
        uint32_t _start;
};

extern RTClock*         initRTClock(void);