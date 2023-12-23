#include <Arduino.h>
#include "Constants.h"
#include "Config.h"
#include "Debug.h"

const static String configFilename("/countdown.json");

#define DEFAULT_TIME_START    "2023-12-12T15:45:00"
#define DEFAULT_TIME_FINAL    "2024-07-28T15:45:00"

#define DEFAULT_MESSAGE_START "YuriCloc"
#define DEFAULT_MESSAGE_FINAL "Good Luc"

#define DEFAULT_AP_SSID       "YurijClock"
#define DEFAULT_AP_PASSWORD   "12345678"

#define DEFAULT_MODE          MODE_COUNTDOWN
#define DEFAULT_BRIGHTNESS    7   //  1 ... 7



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

    _apSSID     = DEFAULT_AP_SSID;
    _apPassword = DEFAULT_AP_PASSWORD;

    _mode       = DEFAULT_MODE;
    _brightness = DEFAULT_BRIGHTNESS;

    memset(_formats,0,sizeof(_formats));

    _brightness = DEFAULT_BRIGHTNESS;

    FILESYSTEM.begin();
}

const String& Config::getFilename(void) { return configFilename;}

int   Config::getMode(void) { return _mode;}
int   Config::getNextMode(void) { return (_mode+1)%N_MODES; }
void  Config::setMode(int mode) { 
  P(NAME); P("setMode"); P("changing mode from");
  P(modeNames[_mode]);P("-->");PL(modeNames[mode]);
  _mode = mode; 
}

int   Config::getFormat(int mode) { return _formats[mode]; }
int   Config::getFormat(void) { return _formats[_mode]; }
int   Config::getNextFormat(void) {
  int format = _formats[_mode];
  format++;
  switch(_mode) {
    case MODE_COUNTDOWN : return format % N_FORMAT_COUNTDOWN; break;
    case MODE_COUNTUP   : return format % N_FORMAT_COUNTUP; break;
    case MODE_CLOCK     : return format % N_FORMAT_CLOCK; break;
    case MODE_TEXT      : return format % N_FORMAT_TEXT; break;
    default: return 0;
  }
}
void  Config::setFormat(int format) { 
  P(NAME); P("setFormat"); P("changing format from");
  P(_formats[_mode]); P("-->"); PL(format);
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

uint8_t Config::getBrightness(void) { return _brightness;}
void    Config::setBrightness(uint8_t brightness) { _brightness = brightness; }

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