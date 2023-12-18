#include <Arduino.h>
#include "Constants.h"
#include "Config.h"
#include "Debug.h"

#define DEFAULT_FUTURE  "2023-12-22T15:45:00"
#define DEFAULT_MESSAGE "YuriCloc"
#define DEFAULT_BRIGHTNESS 7   //  1 ... 7

static  const char* modeNames[N_MODES] = {
  "Countdown", "Clock", "Message", "Demo",
};

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
void  Config::setMode(int mode) { _prevMode = _mode; _mode = mode; }
int   Config::restoreMode(void) { _mode = _prevMode; return _mode; }
void  Config::incMode(void) { 
  int mode = getMode()+1;
  setMode(mode%N_MODES);
  P("changing mode to ");PL(modeNames[_mode]);
}

int   Config::getFormat(void) { return _formats[_mode]; }
int   Config::getFormat(int mode) { return _formats[mode]; }
void  Config::setFormat(int format,int mode) { _formats[mode] = format; }
void  Config::incFormat(void) {
  int format = getFormat(_mode);
  format++;
  switch(_mode) {
    case MODE_COUNTDOWN : _formats[MODE_COUNTDOWN] = format % 7; break;
    case MODE_CLOCK   :  _formats[MODE_CLOCK]      = format % 11; break;
    case MODE_DEMO    :  _formats[MODE_DEMO]       = format % 2; break;
    case MODE_MESSAGE :  _formats[MODE_MESSAGE]    = format % 2; break;
    default: break;
  }
}
bool  Config::isTenthSecFormat(void) {
  bool rc;
  int format = _formats[_mode];
  switch(_mode) {
    case MODE_COUNTDOWN : rc = (format==0) || (format==3); break;
    case MODE_CLOCK :     rc = (format==5) || (format==9); break;
    default : rc = false;
  }
  return rc;
}

uint8_t Config::getBrightness(void) { return _brightness;}
void    Config::setBrightness(uint8_t brightness) { _brightness = brightness;}

void  Config::setText(const String& text) { _text = text; }
String& Config::getText(void) { return _text; }


void  Config::print(void) const {
  P("config:"); 
  SPACE; PVL(_text);
  SPACE; PVL(_future);
  SPACE; Serial.printf("_brightness=0x%x\n",_brightness);

  SPACE; PV(_mode);
  for(int i=0;i<N_MODES;i++) {
    P(",format["); P(i); P("]=");P(_formats[i]);
  }
  PL("");
}