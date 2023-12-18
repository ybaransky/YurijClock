#include "Timer.h"

/*
**********************************************************************
******************** RTTimer ****************************************
**********************************************************************
*/
void Timer::start(ulong period, ulong duration) {
    _period = period; // ms
    _count = 0;
    _start = millis();
    _last  = _start;
    _duration = duration;   // end=0 means its never done, this is ms from _start
    _id    = _period;
  Serial.printf("Timer:%ld starting @ %ld period=%ld duration=%ld\n",_id,_start,_period,_duration);
}
void Timer::stop(void) { 
  Serial.printf("Timer:%ld stopping @ %ld period=%ld duration=%ld actual=%ld\n",_id,millis(),_period,_duration,millis()-_start);
    _duration = 0;
    _period = 0;
    _count = 0;
}

bool Timer::tick(void) {
  if (_period)  {
    uint32_t now = millis();
    if ((now - _last) > _period) {
      _last = now;
      _count++;
      return true;
    }
  }
  return false;
}

int Timer::count(void) {
    return _count;
}

bool Timer::finished(ulong now) {
  if (!_duration) return false ;
  if (!now) now = millis();
  if (now > (_start + _duration))
    return true;
  return false;
}

bool Timer::active(void) {return _period != 0;}

void Timer::print(const char* msg) {
  if (msg) {
    P(msg); SPACE;
  }
  Serial.printf("Timer:%ld state @ %ld period=%ld duration=%ld count=%d\n",_id,millis(),_period,_duration,_count);
}