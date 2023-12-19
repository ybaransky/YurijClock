#include <Arduino.h>
#include "Constants.h"
#include "Config.h"
#include "Debug.h"

const static String configFilename("/coundown.json");

#define DEFAULT_TIME_START    "2023-12-22T15:45:00"
#define DEFAULT_TIME_FINAL      "2023-12-22T15:45:00"

#define DEFAULT_MESSAGE_START "YuriCloc"
#define DEFAULT_MESSAGE_FINAL "Good Luc"

#define DEFAULT_BRIGHTNESS 7   //  1 ... 7

static const char*  NAME="Config::";

Config* initConfig(void) {
    Config* cfg = new Config();
    cfg->init();
    return cfg;
}

void    Config::init(void) {
    _timeStart = DEFAULT_TIME_START;
    _timeFinal = DEFAULT_TIME_FINAL;

    _msgStart = DEFAULT_MESSAGE_START;
    _msgFinal = DEFAULT_MESSAGE_FINAL;

    _mode   = MODE_COUNTDOWN;
    memset(_formats,0,sizeof(_formats));

    _brightness = DEFAULT_BRIGHTNESS;
}

const String& Config::getFilename(void) { return configFilename;}

int   Config::getMode(void) { return _mode;}
int   Config::getNextMode(void) { return (_mode+1)%N_MODES; }
void  Config::setMode(int mode) { 
  P(NAME); P("setMode"); P("changing mode from");
  P(modeNames[_mode]);P("-->");PL(modeNames[mode]);
  _mode = mode; 
}

int   Config::getFormat(void) { return _formats[_mode]; }
int   Config::getNextFormat(void) {
  int format = _formats[_mode];
  format++;
  switch(_mode) {
    case MODE_COUNTDOWN : return format % 7; break;
    case MODE_COUNTUP   : return format % 7; break;
    case MODE_CLOCK     : return format % 11; break;
    case MODE_TEXT      : return format % 2; break;
    default: return 0;
  }
}
void  Config::setFormat(int format) { 
  P(NAME); P("setFormat"); P("changing mode from");P(_formats[_mode]);P("-->");PL(format);
  _formats[_mode] = format;
}

bool  Config::isTenthSecFormat(void) {
  bool rc;
  int format = _formats[_mode];
  switch(_mode) {
    case MODE_COUNTUP : 
    case MODE_COUNTDOWN : return (format==0) || (format==3); 
    case MODE_CLOCK :     return (format==5) || (format==9);
    default : return false;
  }
  return rc;
}

void  Config::print(void) const {
  P("config:"); 
  PVL(_msgStart);
  PVL(_msgFinal);
  PVL(_timeStart);
  PVL(_timeFinal);
  Serial.printf("_brightness=0x%x\n",_brightness);

  PV(_mode);
  for(int i=0;i<N_MODES;i++) {
    P(",format["); P(i); P("]=");P(_formats[i]);
  }
  PL("");
}

void  Config::saveFile(void) {
  PL("trying to save config file");
}