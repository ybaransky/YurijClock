#include "Timer.h"

/*
**********************************************************************
******************** RTTimer ****************************************
**********************************************************************
*/
void Timer::start(uint32_t interval, uint32_t duration) {
    _interval = interval; // ms
    _count = 0;
    _start = millis();
    _last  = _start;
    _duration = duration;   // end=0 means its never done, this is ms from _start

}
void Timer::stop(void) { 
    _interval = 0;
    _count = 0;
}

bool Timer::tick(void) {
  if (_interval)  {
    uint32_t now = millis();
    if ((now - _last) > _interval) {
      _last = now;
      _count++;
      return true;
    }
  }
  return false;
}

uint32_t Timer::count(void) {
    return _count;
}

bool Timer::finished(uint32_t now) {
  if (!_duration) return false ;
  if (!now) now = millis();
  if (now > (_start + _duration))
    return true;
  return false;
}