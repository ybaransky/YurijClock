#pragma once
#include <RTClib.h>
#include "Constants.h"
#include "Segment.h"

#define SEGMENT_BRIGHTEST 7
#define SEGMENT_DIMMEST   1
#define SEGMENT_ON        true
#define SEGMENT_OFF       false
#define SEGMENT_COLON     0x40

class DisplayMsg {
  public:
      void          set(const String&  msg, bool blink=false);
      bool          blink(void) { return  _blink;}
      const String& text(void) const { return _text; }
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
    void    test(void);

    void    setFormat(int);
    int     getFormat(void)   { return _formats[_mode]; }
    void    incFormat(void);


    void    setMode(int);
    int     getMode(void)     { return _mode;};
    void    incMode(void);
    void    restoreMode(void);

    void    showInteger(int32_t);
    void    setBrightness(uint8_t brightness,bool on=true);

    void    showTime(const TimeSpan&, uint8_t=0);
    void    showTime(const DateTime&, uint8_t=0);
    void    showMessage(const DateTime&, const DisplayMsg&);
    void    refresh();

  private:
    void      refresh(const TimeSpan&, uint8_t);
    void      refresh(const DateTime&, uint8_t);
    void      refresh(const DateTime&, const DisplayMsg&);

    Segment   _segments[N_SEGMENTS];
    char      _message[13];
    int       _formats[N_MODES];
    int       _mode,_prevMode;  // count down, count up, religious, ...

    // cache
    Cache     _cache;
};
extern Display* initDisplay(void);