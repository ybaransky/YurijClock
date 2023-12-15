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
    strncpy(_future, DEFAULT_FUTURE, N_ELETS(_future));
    strncpy(_message, DEFAULT_MESSAGE, N_ELETS(_message));
    memset(_formats,0,sizeof(_formats));
    _mode = MODE_COUNTDOWN;
    _brightness = DEFAULT_BRIGHTNESS;

}

int   Config::getMode(void) { return _mode;}
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
    case MODE_COUNTUP : format = _formats[MODE_COUNTUP]; break;
    default: format = 0; break;
  }
  return format;
}

void  Config::setFormat(int format) { 
  switch(_mode) {
    case MODE_COUNTDOWN : _formats[MODE_COUNTDOWN] = format; break;
    case MODE_COUNTUP :  _formats[MODE_COUNTUP] = format; break;
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
    case MODE_COUNTUP :  
      _formats[MODE_COUNTUP] = format % 11; 
      break;
    default: break;
  }
}

uint8_t Config::getBrightness(void) { return _brightness;}
void    Config::setBrightness(uint8_t brightness) { _brightness = brightness;}

void  Config::print(void) const {
  P("config:"); 
  SPACE; PV(_mode);
  SPACE; PV(_formats[0]);
  SPACE; PV(_formats[1]);
  SPACE; PV(_message);
  SPACE; PV(_future);
  SPACE; Serial.printf("0x%x",_brightness);
  PL("");
}