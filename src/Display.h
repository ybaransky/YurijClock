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
  struct Cache {
    TimeSpan  _ts;
    uint8_t   _ms100;
    void      save(TimeSpan ts, uint8_t ms100) { _ts=ts; _ms100 = ms100;}
  };

  public:
    void    init(void);
    void    test(void);

    void    setFormat(int);
    void    setBrightness(uint8_t brightness,bool on=true);
    void    showInteger(int32_t);
    void    showTimeSpan(TimeSpan,uint8_t=0);
    void    refresh(TimeSpan,uint8_t);

  private:
    Segment   _segments[N_SEGMENTS];
    int       _times[N_ELEMENTS];   // DAYS,HOURS,MINUTES,SECONDS,MILLIS
    char      _message[13];
    int       _format;

    // cache
    Cache     _cache;
};

extern Display* initDisplay(void);