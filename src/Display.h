#pragma once
#include "Segment.h"

typedef union {
  char     chars[12];
  int8_t   bytes[12];
  int32_t  ints[3];
} DisplayBuffer;

class Display {
    static const int N_SEGMENTS = 3;
    public:
        void    init(void);
        void    setBrightness(uint8_t);
        void    test();
        void    showInteger(int32_t);

    private:
        Segment _segments[N_SEGMENTS];
};