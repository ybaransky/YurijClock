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
*  DisplayMessage Class
*************************************************************************
*/

void DisplayMsg::set(const String& text, bool blink) {
  _text  = text;
  _blink = blink;
}

/*
*************************************************************************
*  Display Class
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
void  Display::setMode(int mode) { 
  _prevMode = _mode; _mode = mode;
    P("setting mode to "); PL(_mode);
}
void  Display::restoreMode(void) { 
  _mode = _prevMode; 
  P("restoring mode to "); PL(_mode);
}
void  Display::incMode(void) {
  int mode = getMode();
  mode++;
  mode = mode % 2;
  setMode(mode);
}
void  Display::setFormat(int format)  {_formats[_mode] = format;}
void  Display::incFormat(void) {
  int format = getFormat();
  format++;
  format = (getMode()==MODE_COUNTDOWN) ? format % 7 : format % 12; 
  setFormat(format);
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

void Display::showTime(const TimeSpan& ts, uint8_t ms100) {
  _cache.save(ts,ms100);
  refresh(ts,ms100);
}

void Display::showTime(const DateTime& dt, uint8_t ms100) {
  _cache.save(dt,ms100);
  refresh(dt,ms100);
}

void Display::refresh(const TimeSpan& ts, uint8_t ms100) {
  int format = getFormat();
  _segments[DDDD].drawDDDD(ts,format);
  _segments[HHMM].drawHHMM(ts,format);
  _segments[SSUU].drawSSUU(ts,ms100,format);
}

void  Display::refresh(const DateTime& dt, uint8_t ms100) {
  int format = getFormat();
  _segments[DDDD].drawDDDD(dt,format);
  _segments[HHMM].drawHHMM(dt,format);
  _segments[SSUU].drawSSUU(dt,ms100,format);
}

void  Display::refresh(const DateTime& dt, const DisplayMsg& dmsg) {
  char buffer[13];
  snprintf(buffer,13,"%12s",dmsg.text().c_str());
  
  _segments[DDDD].drawText(dt,&buffer[0]);
  _segments[HHMM].drawText(dt,&buffer[4]);
  _segments[SSUU].drawText(dt,&buffer[8]);
}

void Display::refresh(void) {
  switch(getMode()) {
    case MODE_COUNTDOWN :  refresh(_cache.ts(), _cache.ms()); break;
    case MODE_COUNTUP :    refresh(_cache.dt(), _cache.ms()); break;
    case MODE_MESSAGE :    refresh(_cache.dt(), _cache.displayMsg()); break;
  }
}

void Display::showMessage(const DateTime& dt, const DisplayMsg& msg) {
  _cache.save(dt,msg);

}
