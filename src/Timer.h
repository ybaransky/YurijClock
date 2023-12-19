#pragma once
#include <Arduino.h>

class Timer {
    public:
        void        start(ulong period);
        void        reset(void);
        bool        tick(void);
        int         count(void); 

        void        print(const char *msg);

    private:
        ulong    _period;
        ulong    _id;
        int      _count;
        ulong    _last;
        ulong    _start;
};