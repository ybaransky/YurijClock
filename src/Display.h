#pragma once
#include <RTClib.h>
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
    DateTime  _dt;
    uint8_t   _ms;  // 100 ms increments
    TimeSpan  ts(void) {return _ts;}
    DateTime  dt(void) {return _dt;}
    uint8_t   ms(void) {return _ms;}
    void      save(TimeSpan ts, uint8_t ms) { _ts=ts; _ms = ms;}
    void      save(DateTime dt, uint8_t ms) { _dt=dt; _ms = ms;}
  };

  public:
    void    init(void);
    void    test(void);

    void    setFormat(int);
    void    incFormat(void);
    int     getFormat(void) const;


    void    setMode(int);
    void    incMode(void);
    int     getMode(void) const;

    void    showInteger(int32_t);
    void    setBrightness(uint8_t brightness,bool on=true);

    void    showTime(TimeSpan,uint8_t=0);
    void    showTime(DateTime,uint8_t=0);
    void    showMessage(char*, bool on);
    void    refresh();

  private:
    void      refresh(TimeSpan,uint8_t);
    void      refresh(DateTime,uint8_t);

    Segment   _segments[N_SEGMENTS];
    char      _message[13];
    int       _formats[N_MODES];
    int       _mode;  // count down, count up, religious, ...

    // cache
    Cache  _cache;
};

extern Display* initDisplay(void);