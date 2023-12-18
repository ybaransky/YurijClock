#pragma once
#include <Arduino.h>

class Timer {
    public:
        void        start(ulong period, ulong duration=0);
        void        stop(void);
        bool        tick(void);
        bool        active(void);
        bool        finished(ulong now=0);
        int         count(void); 

        void        print(const char *msg);

    private:
        ulong    _period;
        ulong    _id;
        int      _count;
        ulong    _last;
        ulong    _start;
        ulong    _duration;
};

