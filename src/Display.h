#pragma once
#include <RTCLib.h>
#include "Constants.h"
#include "Segment.h"

#define SEGMENT_BRIGHTEST 7
#define SEGMENT_DIMMEST   1
#define SEGMENT_ON        true
#define SEGMENT_OFF       false
#define SEGMENT_COLON     0x40
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