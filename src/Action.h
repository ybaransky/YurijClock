#pragma once
#include <Arduino.h>
#include <RTClib.h>

class Action {
  public:
    enum Type {
      UNKNOWN, SPLASH, DEMO, FINAL
    };
    Action() : _active(false) {}
    void      splash(const String& msg, int prevMode, ulong duration=5000);
    void      demo(const String& msg, int prevMode, const DateTime&);
    void      start(Type id, const String& msg, int prevMode, ulong duration=5000, bool blinking=false);

    void      stop(void);
    bool      active(void) { return _active;}
    bool      expired(ulong ms=0); 
    void      print(const char* msg=nullptr);

    int             getPrevMode(void) { return _prevMode;}
    const String&   getMsg(void) { return _msg;}
    const DateTime& getExpireTime(void) { return _expireTime;}
    bool            isBlinking(void) { return _blinking;}

    int         _prevMode;      
    DateTime    _prevDateTime;  // used to save origanl countdim timw
    
    ulong       _start;     
    ulong       _duration;      // demo mode uses this to countdown, and this to blink the msg
    String      _msg;

    bool        _active;
    bool        _blinking;
    Type        _type;   // splash, demo, final

    DateTime    _expireTime;  // time the coundown goes to zero
};