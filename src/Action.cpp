#include "Action.h"

void  Action::splash(const String& msg, int prevMode, ulong duration) {
  start(Action::SPLASH, msg, prevMode, duration, false);
}
void  Action::demo(const String& msg, int prevMode, const DateTime& now ) {
  _expireTime = now + TimeSpan(5) ;
  start(Action::DEMO, msg, prevMode, 10000, true);  // 5 seconds for coundown, 5 for blinking
}
void  Action::start(Type type, const String& msg, int prevMode, ulong duration, bool blinking) {
  _start    = millis();
  _prevMode = prevMode;
  _msg      = msg; 
  _duration = duration;
  _active   = true;
  _type     = type;
  _blinking = blinking;
  print("action-start ");
};

void  Action::print(const char* msg) {
  if (msg) {P(msg); SP;}
  PV(_type); SP; PV(_msg); SP; PV(_duration); SP; PV(_prevMode); SP;
  PV(_active); SP; PV(_blinking);  
  PL("");
}

void  Action::stop(void) {_active = false;}

bool  Action::expired(ulong now) {
  if (_active) {
    if (!now) 
      now = millis();
    return now - _start > _duration;
  }
  return true;
}