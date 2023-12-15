#pragma once
#include <Arduino.h>

class Timer {
    public:
        void        start(uint32_t interval, uint32_t duration=0);
        void        stop(void);
        bool        tick(void);
        bool        finished(uint32_t now=0);
        uint32_t    count(void); 

    private:
        uint32_t    _interval;
        uint32_t    _count;
        uint32_t    _last;
        uint32_t    _start;
        uint32_t    _duration;
};

