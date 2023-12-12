#include <Arduino.h>
#include "Display.h"
#include "Constants.h"
#include "Debug.h"
/*
12/7/2023
For some reason, I can't create the TM1637Display objects via new. 
This needs to be understood 
*/
/*
*************************************************************************
*  Device Class
*************************************************************************
*/

Display* initDisplay(void) {
  Display* display = new Display();
  display->init();
  return display;
}

void Display::init(void) {
  _displayMode = DISPLAY_COUNTDOWN;
  for(int i=0;i < N_DISPLAY_MODES; i++) 
    _formats[i] = 0;
  for(int i=0; i < N_SEGMENTS; i++) {
    _segments[i].init(i,_formats);
    _segments[i].device().clear();
  }
}

void  Display::setBrightness(uint8_t brightness, bool on) {
  for(int i=0;i<N_SEGMENTS;i++) 
    _segments[i].setBrightness(brightness, on);
}

int   Display::getDisplayMode(void) const {return _displayMode;}
void  Display::setDisplayMode(int displayMode) {
    Serial.printf("setting displayMode to %d\n",displayMode);
    _displayMode = displayMode;
    setSegmentModeFormat();
}

int   Display::getFormat(void) const {return _formats[_displayMode];}
void  Display::setFormat(int format) {
    Serial.printf("setting format to %d\n",format);
    _formats[_displayMode] = format;
    setSegmentModeFormat();
}

void  Display::setSegmentModeFormat(void) {
    for(int i=0;i<N_ELEMENTS;i++) 
      _segments[i].setFormat(_formats[_displayMode], _displayMode);
    if (_displayMode==DISPLAY_COUNTDOWN)
      refresh(_cache._ts, _cache._ms100);
    else
      refresh(_cache._dt, _cache._ms100);
}

void Display::test(void) {
  int values[] = {0000,1111,2222};
  bool zeroPad = true;
  bool on=true;
  for(int k=0; k < 2; k++) {
    for(int i=0;i<N_SEGMENTS;i++) {
      _segments[i].device().setBrightness(SEGMENT_BRIGHTEST,on);
      _segments[i].device().showNumberDecEx(values[i], SEGMENT_COLON, zeroPad);
    }
    delay(250);
    on = !on;
  }
  setBrightness(BRIGHTEST);
}

void Display::showInteger(int32_t ival) {
    int parts[3];
    parts[2] =  ival / 100000000;
    parts[1] = (ival % 100000000) / 10000;
    parts[0] = (ival % 10000);
    for(int i=0; i<3; i++)
      _segments[i].device().showNumberDec(parts[i],false);
}

void Display::showTime(TimeSpan ts, uint8_t ms100) {
  _cache.save(ts,ms100);
  refresh(ts,ms100);
}

void Display::showTime(DateTime dt, uint8_t ms100) {
  _cache.save(dt,ms100);
  refresh(dt,ms100);
}

void Display::refresh(TimeSpan ts, uint8_t ms100) {
  _segments[DDDD].drawDDDD(ts);
  _segments[HHMM].drawHHMM(ts);
  _segments[SSUU].drawSSUU(ts,ms100);
}

void  Display::refresh(DateTime dt, uint8_t ms100) {
  _segments[DDDD].drawDDDD(dt);
  _segments[HHMM].drawHHMM(dt);
  _segments[SSUU].drawSSUU(dt,ms100);
}
