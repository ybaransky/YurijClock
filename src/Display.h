#pragma once
#include <RTClib.h>
#include "Constants.h"
#include "Segment.h"

class Display {

  struct Cache {
    TimeSpan        _ts;
    DateTime        _dt;
    String          _msg;
    uint8_t         _tenth;  // 100 ms increments

    const TimeSpan&   ts(void) {return _ts;}
    const DateTime&   dt(void) {return _dt;}
    const String &    msg(void) {return _msg;}
    uint8_t           tenth(void) {return _tenth;}

    void   save(const TimeSpan& ts, uint8_t tenth) { _ts=ts; _tenth = tenth;}
    void   save(const DateTime& dt, uint8_t tenth) { _dt=dt; _tenth = tenth;}
    void   save(const String& msg) { _msg = msg;}
  };

  public:
    void    init(void);
    void    clear(void);

    void    test(void);

    void    showInteger(int32_t);
    void    setBrightness();

    void    showCount(const TimeSpan&, uint8_t tenth=0);
    void    showClock(const DateTime&, uint8_t tenth=0);
    void    showText(const String&, bool visible);

  private:
    void    showCountDDDD(const TimeSpan&);
    void    showCountHHMM(const TimeSpan&);
    void    showCountSSUU(const TimeSpan&, uint8_t tenth);
    
    void    showClockDDDD(const DateTime&);
    void    showClockHHMM(const DateTime&);
    void    showClockSSUU(const DateTime&, uint8_t tenth);

    void    encode(char, char, char, char);
    void    writeSegment(int,bool colon=false,bool visible=true); 

    Segment   _segments[N_SEGMENTS];
    String    _text;
    int       _format;

    // cache
    Cache     _cache;

    uint8_t   _data[4];
};

extern Display* initDisplay(void);