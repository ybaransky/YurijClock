#include "Action.h"

void  Action::start(Type type, const String& message, ulong seconds, bool blinking) {
  _type     = type;
  _message  = message; 
  _duration = 1000 * seconds;
  _blinking = blinking;

  _active   = true;
  _start    = millis();
  if (_type == DEMO) {
    // spend 5 seconds counting down, then the message
    int countdown = 5 * 1000; 
    _duration += countdown;
    _zeroTime = _start + countdown;
  }
  print("action-start ");
};

void  Action::print(const char* msg) {
  if (msg) {P(msg); SPACE;}
  PV(_type); SPACE; PV(_message); SPACE; PV(_duration); P("ms "); SPACE;
  PV(_active); SPACE; PV(_blinking);  
  PL("");
}

void  Action::stop(const char* msg) {
  if (msg) {P(msg); SPACE;}
  PL("action stopping");
  _active = false;
  _type = Type::NONE;
}

bool  Action::isOver(void) {
  if (_active) {
    return (millis() - _start) > _duration;
  }
  return true;
}

int Action::getSecsRemaining(void) const { 
  int secs = (_zeroTime - millis())/1000;
  return (secs >= 0) ? secs : 0;
}

bool Action::showDemoMessage(void) const { 
  return millis() > _zeroTime ? true : false;
}