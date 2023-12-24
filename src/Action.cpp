#include "Action.h"
#include "Config.h"

void  Action::splash(const String& msg, ulong duration) {
  start(Action::SPLASH, msg, duration, false);
  config->setMode(MODE_TEXT);
}
void  Action::demo(const String& msg, const DateTime& now ) {
  _expireTime = now + TimeSpan(5) ;
  start(Action::DEMO, msg, 10000, true);  // 5 seconds for coundown, 5 for blinking
  config->setMode(MODE_COUNTDOWN);
  config->setFormat(0);
}

void  Action::info(const String& msg, const DateTime& now ) {
  start(Action::INFO, msg, 5000, false);  // 5 seconds for info
  config->setMode(MODE_TEXT);
}

void  Action::start(Type type, const String& msg, ulong duration, bool blinking) {
  _start    = millis();
  _prevMode   = config->getMode();
  _prevFormat = config->getFormat();
  _msg      = msg; 
  _duration = duration;
  _active   = true;
  _type     = type;
  _blinking = blinking;
  print("action-start ");
};

void  Action::print(const char* msg) {
  if (msg) {P(msg); SPACE;}
  PV(_type); SPACE; PV(_msg); SPACE; PV(_duration); SPACE; PV(_prevMode); SPACE;
  PV(_active); SPACE; PV(_blinking);  
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

void  Action::setPrevDisplay(void) {
  config->setMode(_prevMode);
  config->setFormat(_prevFormat);
}