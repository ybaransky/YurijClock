#pragma once
#include <RTClib.h>
#include "Constants.h"
#include "Segment.h"
#include "Message.h"

class Display {

  struct Cache {
    TimeSpan        _ts;
    DateTime        _dt;
    uint8_t         _ms;  // 100 ms increments
    Message         _msg;

    const TimeSpan&   ts(void) {return _ts;}
    const DateTime&   dt(void) {return _dt;}
    uint8_t           ms(void) {return _ms;}
    const Message&    msg(void) {return _msg;}

    void   save(const TimeSpan& ts, uint8_t ms) { _ts=ts; _ms = ms;}
    void   save(const DateTime& dt, uint8_t ms) { _dt=dt; _ms = ms;}
    void   save(const Message& msg) { _msg = msg;}
  };

  public:
    void    init(void);
    void    reset(void);

    void    test(void);

    void    showInteger(int32_t);

    void    showCountDown(const TimeSpan&, uint8_t=0);
    void    showClock(const DateTime&, uint8_t=0);
    void    showMessage(const Message&, uint32_t count);
    void    refresh();

  private:
    Segment   _segments[N_SEGMENTS];
    char      _message[13];

    // cache
    Cache     _cache;
};
extern Display* initDisplay(void);