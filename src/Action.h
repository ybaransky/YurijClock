#pragma once
#include <Arduino.h>
#include <RTClib.h>

class Action {
  public:
    enum Type {
      NONE, INFO, DEMO, FINAL
    };
    Action() : _active(false) {}
    void      startDemo(void);
    void      startInfo(const String& msg, ulong seconds);
    void      start(Type id, const String& msg, ulong seconds, bool blinking=false);

    void      stop(void);
    void      tick(void);
    bool      expired(void);
    bool      active(void) { return _active;}
    void      print(const char* msg=nullptr);

    void            restore(void);
    const String&   getMsg(void) { return _msg;}
    bool            isBlinking(void) { return _blinking;}
    bool            isDemoMode(void) { return _type == Type::DEMO;}
    int             getSecsRemaining(void);

    int         _prevMode; 
    int         _prevFormat;
    DateTime    _prevDateTime;  // used to save origanl countdim timw
    
    ulong       _now;
    ulong       _start;
    ulong       _duration;      // demo mode uses this to countdown, and this to blink the msg
    String      _msg;

    bool        _active;
    bool        _blinking;
    Type        _type;   // splash, demo, final

    ulong       _zeroTime;  // millis from start when demo should flip
};