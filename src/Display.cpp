#include <Arduino.h>
#include "Display.h"
#include "Config.h"
#include "Constants.h"
#include "Debug.h"
/*
12/7/2023
For some reason, I can't create the TM1637Display objects via new. 
This needs to be understood 
*/

#define SEGMENT_BRIGHTEST 7
#define SEGMENT_DIMMEST   1
#define SEGMENT_ON        true
#define SEGMENT_OFF       false
#define SEGMENT_COLON     0x40

/*
*************************************************************************
*  DisplayMessage Class
*************************************************************************
*/

void DisplayMsg::set(const String& text, bool blink) {
  _text  = text;
  _blink = blink;
  P("DisplayMsg: |"); P(_text); PL("|");
}

void  DisplayMsg::print(void) const {
  P("DMsg:");
  SPACE; PV(_text);
  SPACE; PV(_blink);
  PL("");
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
  for(int i=0; i < N_SEGMENTS; i++) 
    _segments[i].init(i);
  reset();
}

void Display::reset(void) {
  uint8_t brightness = config->getBrightness();
  for(auto& segment : _segments) {
    segment.device().clear();
    segment.setBrightness(brightness);
    segment.setVisible(true);
  }
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
  for(auto& segment : _segments)
    segment.device().setBrightness(SEGMENT_BRIGHTEST,true);
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

void Display::showText(const DateTime& dt, const DisplayMsg& dmsg) {
  _cache.save(dt,dmsg);
  refresh(dt,dmsg);
}

void Display::refresh(const TimeSpan& ts, uint8_t ms100) {
  int format = config->getFormat();
  _segments[DDDD].drawDDDD(ts,format);
  _segments[HHMM].drawHHMM(ts,format);
  _segments[SSUU].drawSSUU(ts,ms100,format);
}

void  Display::refresh(const DateTime& dt, uint8_t ms100) {
  int format = config->getFormat();
  _segments[DDDD].drawDDDD(dt,format);
  _segments[HHMM].drawHHMM(dt,format);
  _segments[SSUU].drawSSUU(dt,ms100,format);
}

void  Display::refresh(const DateTime& dt, const DisplayMsg& dmsg) {
  char buffer[13];
  snprintf(buffer,13,"%12s",dmsg.text().c_str());
  
  bool visible = dmsg.isBlinking() ? dt.second() % 2 : true;
  PV("refresh: "); SPACE; 
  PV(visible); SPACE; 
  PV(dmsg.isBlinking()); SPACE;
  PL("");
  _segments[DDDD].drawText(&buffer[0],visible);
  _segments[HHMM].drawText(&buffer[4],visible);
  _segments[SSUU].drawText(&buffer[8],visible);
}

void Display::refresh(void) {
  switch(config->getMode()) {
    case MODE_COUNTDOWN :  refresh(_cache.ts(), _cache.ms()); break;
    case MODE_COUNTUP :    refresh(_cache.dt(), _cache.ms()); break;
    case MODE_MESSAGE :    refresh(_cache.dt(), _cache.displayMsg()); break;
  }
}
