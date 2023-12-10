#pragma once
#include <RTCLib.h>
#include "Segment.h"

#define SEGMENT_BRIGHTEST 7
#define SEGMENT_DIMMEST   1
#define SEGMENT_ON        true
#define SEGMENT_OFF       false
#define SEGMENT_COLON     0x40

enum  {
  N_SEGMENTS=3, DDDD=2, HHMM=1, SSUU=0,
  N_ELEMENTS=5, DAYS=4, HOURS=3, MINUTES=2, SECONDS=1, MILLIS=0,
  N_DIGITS=12,   // 0=right-most
  N_SEGMENT_NAME=5,
};

class Display {
   public:
    void    init(void);
    void    test(void);

    void    enable(bool);
    void    setBrightness(uint8_t brightness,bool on=true);
    void    showInteger(int32_t);
    void    showTimeSpan(TimeSpan);
    void    showTimeSpan(TimeSpan,uint8_t);

  private:
    void      toTimeArray(TimeSpan ts);
    Segment   _segments[N_SEGMENTS];
    int       _times[N_ELEMENTS];   // DAYS,HOURS,MINUTES,SECONDS,MILLIS
    char      _message[13];
    int       _format;
};

extern Display* initDisplay(void);