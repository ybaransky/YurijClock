#pragma once
#include <RTClib.h>
#include "Constants.h"
#include "Segment.h"
class DisplayMsg {
  public:
      void          set(const String&  msg, bool blink=false);
      const bool&   isBlinking(void) const { return _blink;}
      const String& text(void) const { return _text; }
      void          print(void) const;
  private:
      String  _text;
      bool    _blink;
};

class Display {

  struct Cache {
    TimeSpan        _ts;
    DateTime        _dt;
    uint8_t         _ms;  // 100 ms increments
    DisplayMsg      _displayMsg;

    const TimeSpan&   ts(void) {return _ts;}
    const DateTime&   dt(void) {return _dt;}
    uint8_t           ms(void) {return _ms;}
    const DisplayMsg& displayMsg(void) {return _displayMsg;}
    void              save(TimeSpan ts, uint8_t ms) { _ts=ts; _ms = ms;}
    void              save(DateTime dt, uint8_t ms) { _dt=dt; _ms = ms;}
    void              save(DateTime dt, const DisplayMsg& dmsg) { _displayMsg = dmsg;}
  };

  public:
    void    init(void);
    void    reset(void);

    void    test(void);

    void    showInteger(int32_t);

    void    showTime(const TimeSpan&, uint8_t=0);
    void    showTime(const DateTime&, uint8_t=0);
    void    showText(const DateTime&, const DisplayMsg&);
    void    refresh();

  private:
    void      refresh(const TimeSpan&, uint8_t);
    void      refresh(const DateTime&, uint8_t);
    void      refresh(const DateTime&, const DisplayMsg&);

    Segment   _segments[N_SEGMENTS];
    char      _message[13];

    // cache
    Cache     _cache;
};
extern Display* initDisplay(void);