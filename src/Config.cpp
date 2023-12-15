#include <Arduino.h>
#include "Constants.h"
#include "Config.h"
#include "Debug.h"

#define DEFAULT_FUTURE  "2023-12-22T15:45:00"
#define DEFAULT_MESSAGE "YuriCloc"
#define DEFAULT_BRIGHTNESS 7   //  1 ... 7

Config* initConfig(void) {
    Config* cfg = new Config();
    cfg->init();
    return cfg;
}

void    Config::init(void) {
    _future = DEFAULT_FUTURE;
    _text   = DEFAULT_MESSAGE;
    _mode   = MODE_COUNTDOWN;
    _brightness = DEFAULT_BRIGHTNESS;
    memset(_formats,0,sizeof(_formats));
}

int   Config::getMode(void) { return _mode;}
void  Config::incMode(void) { 
  int mode = getMode()+1;
  P("changing moe to ");PVL(mode);
  setMode(mode%3);
}
void  Config::setMode(int mode) { 
  _prevMode = _mode;
  _mode = mode; 
}
int   Config::restoreMode(void) { 
  _mode = _prevMode;
  return _mode;
}

int   Config::getFormat(void) { 
  int format;
  switch(_mode) {
    case MODE_COUNTDOWN : format = _formats[MODE_COUNTDOWN]; break;
    case MODE_CLOCK :     format = _formats[MODE_CLOCK]; break;
    case MODE_MESSAGE :   format = _formats[MODE_MESSAGE]; break;
    default: format = 0; break;
  }
  return format;
}

void  Config::setFormat(int format) { 
  switch(_mode) {
    case MODE_COUNTDOWN : _formats[MODE_COUNTDOWN] = format; break;
    case MODE_CLOCK :     _formats[MODE_CLOCK] = format; break;
    case MODE_MESSAGE :   _formats[MODE_MESSAGE] = format; break;
    default: break;
  }
}

void  Config::incFormat(void) {
  int format = getFormat();
  format++;
  switch(_mode) {
    case MODE_COUNTDOWN : 
      _formats[MODE_COUNTDOWN] = format % 7; 
      break;
    case MODE_CLOCK :  
      _formats[MODE_CLOCK] = format % 11; 
      break;
    case MODE_MESSAGE :  
      _formats[MODE_MESSAGE] = format % 2; 
      break;
    default: break;
  }
}

uint8_t Config::getBrightness(void) { return _brightness;}
void    Config::setBrightness(uint8_t brightness) { _brightness = brightness;}

void  Config::setText(const String& text) { _text = text; }
String& Config::getText(void) { return _text; }


void  Config::print(void) const {
  P("config:"); 
  SPACE; PV(_mode);
  SPACE; PV(_formats[0]);
  SPACE; PV(_formats[1]);
  SPACE; PV(_formats[2]);
  SPACE; PV(_text);
  SPACE; PV(_future);
  SPACE; Serial.printf("0x%x",_brightness);
  PL("");
}