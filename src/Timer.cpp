#include "Timer.h"

/*
**********************************************************************
******************** RTTimer ****************************************
**********************************************************************
*/
void Timer::start(ulong period) {
  _period = period; // ms
  reset();
  Serial.printf("Timer:%ld starting @ %ld period=%ld\n",_period,_start,_period);
}

void  Timer::reset(void) {
  _start = millis();
  _last  = _start;
  _count = 0;
}

bool Timer::tick(void) {
  if (!_period) return false;

  uint32_t now = millis();
  if ((now - _last) > _period) {
    _last = now;
    _count++;
    return true;
  }
  return false;
}

int Timer::count(void) {
    return _count;
}

void Timer::print(const char* msg) {
  if (msg) {
    P(msg);
  }
  Serial.printf("Timer:%ld state @ %ld count=%d\n",_period, millis(),_count);
}