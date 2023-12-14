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
  _mode = MODE_COUNTDOWN;
  for(int i=0;i < N_MODES; i++) 
    _formats[i] = 0;
  for(int i=0; i < N_SEGMENTS; i++) {
    _segments[i].init(i);
    _segments[i].device().clear();
  }
}

void  Display::setBrightness(uint8_t brightness, bool on) {
  for(int i=0;i<N_SEGMENTS;i++) 
    _segments[i].setBrightness(brightness, on);
}

int   Display::getMode(void) const {return _mode;}
void  Display::incMode(void) {
  int mode = getMode();
  mode++;
  mode = mode % 2;
  setMode(mode);
}
void  Display::setMode(int mode) {
    _mode = mode;
}

int   Display::getFormat(void) const {return _formats[_mode];}
void  Display::incFormat(void) {
  int format = getFormat();
  format++;
  format = (getMode()==MODE_COUNTDOWN) ? format % 7 : format % 12; 
  setFormat(format);
}
void  Display::setFormat(int format) {
    _formats[_mode] = format;
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

void Display::showMessage(char* message, bool on) {

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
  int format = getFormat();
  _segments[DDDD].drawDDDD(ts,format);
  _segments[HHMM].drawHHMM(ts,format);
  _segments[SSUU].drawSSUU(ts,ms100,format);
}

void  Display::refresh(DateTime dt, uint8_t ms100) {
  int format = getFormat();
  _segments[DDDD].drawDDDD(dt,format);
  _segments[HHMM].drawHHMM(dt,format);
  _segments[SSUU].drawSSUU(dt,ms100,format);
}

void Display::refresh(void) {
  switch(getMode()) {
    case MODE_COUNTDOWN :  refresh(_cache.ts(), _cache.ms()); break;
    case MODE_COUNTUP :    refresh(_cache.dt(), _cache.ms()); break;
  }
}
