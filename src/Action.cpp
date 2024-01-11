#include "Action.h"
#include "Config.h"

void  Action::startInfo(const String& msg, ulong seconds) {
  start(Action::INFO, msg, seconds, false);  // 5 seconds for info
  config->setMode(MODE_TEXT);
}

void  Action::startDemo(void) {
  start(Action::DEMO, config->getMsgEnd(), 10, true); 
  _zeroTime = _start + 5000; // 5 seconds for coundown, 5 for blinking
  config->setMode(MODE_DEMO);
  config->setFormat(0);
}

void  Action::start(Type type, const String& msg, ulong seconds, bool blinking) {
  _start    = millis();
  _prevMode   = config->getMode();
  _prevFormat = config->getFormat();
  _msg      = msg; 
  _duration = seconds * 1000;
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

void  Action::stop(void) {
  _active = false;
  _type = Type::NONE;
}

void  Action::tick(void) {
  _now = millis();
}

bool  Action::expired(void) {
  if (_active) {
    return _now - _start > _duration;
  }
  return true;
}

int Action::getSecsRemaining(void) { 
  return (_zeroTime > _now) ? (_zeroTime - _now)/1000 : -1;
}

void  Action::restore(void) {
  config->setMode(_prevMode);
  config->setFormat(_prevFormat);
}