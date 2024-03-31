#pragma once
#include <Arduino.h>
#include <RTClib.h>

class Action {
  public:
    enum Type {
      NONE, TEXT, DEMO, ADDR, FINAL
    };
    Action() : _active(false) {}
    void      start(Type id, const String& msg, ulong seconds, bool blinking=false);

    void      stop(const char* msg=nullptr);
    Type      type(void) { return _type;}
    bool      active(void) { return _active;}
    void      print(const char* msg=nullptr);
    ulong     startTime(void) const {return _start;}

    const String&   getMessage(void) { return _message;}
    bool            isOver(void);
    bool            isBlinking(void) { return _blinking;}
    bool            isDemoMode(void) { return Type::DEMO == _type;}

    bool      showDemoMessage(void) const;
    int       getSecsRemaining(void) const;

    Type        _type;       // splash, demo, final
    String      _message;
    ulong       _duration;   //  total time (demo is with 1/2 countdown)
    bool        _blinking;
    
    ulong       _start;
    ulong       _zeroTime;  // millis from start when demo should flip

    bool        _active;

  //  DateTime    _prevDateTime;  // used to save origanl countdim timw
};