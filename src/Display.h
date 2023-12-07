#pragma once
#include <TM1637Display.h>

typedef TM1637Display Segment;
typedef TM1637Display* SegmentPtr;

class Display {
    static const int N_SEGMENTS=3;


    public:
        void    init(void);
        void    setBrightness(uint8_t);
        void    test();

        Segment& device(int i); 
};