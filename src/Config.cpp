#include <Arduino.h>
#include "Constants.h"
#include "Config.h"
#include "Debug.h"

const static String cfgFileName("/countdown.json");
#define FILE_WRITE  "w"
#define FILE_READ   "r"
#define JSON_DOC_SIZE 512
#define ISOTIME_SIZE 16
#define SSID_SIZE 16
#define PASSWORD_SIZE 16

//                           1234567890123456
#define DEFAULT_TIME_START  "2023-12-22T14:45"    // no seconds
#define DEFAULT_TIME_END    "2024-07-28T15:45"

#define DEFAULT_MESSAGE_START "AlexiCloc"
#define DEFAULT_MESSAGE_END   "Good Luc"

#define DEFAULT_AP_SSID       "AlexClock"
#define DEFAULT_AP_PASSWORD   "12345678"

#define DEFAULT_MODE          MODE_COUNTDOWN
#define DEFAULT_FORMAT        0
#define DEFAULT_BRIGHTNESS    7   //  1 ... 7

static const char*  NAME="Config::";

Config* initConfig(void) {
    FILESYSTEM.begin();
    Config* cfg = new Config();
    cfg->init();
    if (!cfg->loadFile()) 
      cfg->saveFile();
    return cfg;
}

void    Config::init(void) {
  _timeStart  = DEFAULT_TIME_START;
  _timeEnd    = DEFAULT_TIME_END;

  _msgStart   = DEFAULT_MESSAGE_START;
  _msgEnd     = DEFAULT_MESSAGE_END;

  _apSSID     = DEFAULT_AP_SSID;
  _apPassword = DEFAULT_AP_PASSWORD;

  _mode       = DEFAULT_MODE;
  memset(_formats,DEFAULT_FORMAT,sizeof(_formats));

  _brightness = DEFAULT_BRIGHTNESS;
}

const String& Config::getFileName(void) const { return cfgFileName;}

int   Config::getMode(void) { return _mode;}
int   Config::getNextMode(void) { return (_mode+1)%N_MODES; }
void  Config::setMode(int mode, const char* caller) { 
  const char* fcn="setMode";
  P(NAME); P(fcn);
  if (caller) { P(" called from "); P(caller);}
  SPACE; P(modeNames[_mode]);P(" --> ");PL(modeNames[mode]);
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
  P(NAME); P("setFormat"); P(" changing format ");
  P(_formats[_mode]); P("-->"); PL(format);
  _formats[_mode] = format;
}
void  Config::setFormat(int format, int mode) { 
  P(NAME); P("setFormat(format,mode)"); P(" changing format ");
  P(_formats[mode]); P(" --> "); PL(format);
  _formats[mode] = format;
}

bool  Config::isTenthSecFormat(void) {
  bool rc;
  int format = _formats[_mode];
  switch(_mode) {
    case MODE_COUNTUP : 
    case MODE_COUNTDOWN : return (format==0) || (format==4); 
    case MODE_CLOCK :     return (format==5) || (format==9);
    default : return false;
  }
  return rc;
}

const String& Config::getSSID(void) { return _apSSID;}
void          Config::setSSID(const String& ssid) {_apSSID = ssid;} 
const String& Config::getPassword(void) { return _apPassword;}
void          Config::setPassword(const String& password) {_apPassword = password;} 

uint8_t Config::getBrightness(void) { return _brightness;}
void    Config::setBrightness(uint8_t brightness) { _brightness = brightness; }

static bool changedString(const char* desc, const String& before, const String& after) {
  if (before != after) {
    P("changing "); P(desc); SPACE; P(before); P(" --> "); PL(after);
    return true;
  }
  return false;
}

const String& Config::getTimeStart(void) { return _timeStart;}
void          Config::setTimeStart(const String& time) { 
  if (changedString("timeStart", _timeStart, time))
    _timeStart = time;
}
const String& Config::getTimeEnd(void) { return _timeEnd;}
void          Config::setTimeEnd(const String& time) {
  if (changedString("timeEnd", _timeEnd, time))
    _timeEnd = time;
}

const String& Config::getMsgStart(void) { return _msgStart;}
void          Config::setMsgStart(const String& msg) { 
  if (changedString("msgStart", _msgStart, msg))
    _msgStart = msg;
}
const String& Config::getMsgEnd(void) { return _msgEnd;}
void          Config::setMsgEnd(const String& msg) {
  if (changedString("msgEnd", _msgEnd, msg))
    _msgEnd = msg;
}

void  Config::print(void) const {
  PL("config:"); 
  PVL(_mode);
  PVL(_msgStart);
  PVL(_msgEnd);
  PVL(_timeStart);
  PVL(_timeEnd);
  Serial.printf("_brightness=0x%x\n",_brightness);

  PV(_mode);
  for(int i=0;i<N_MODES;i++) {
    P(",format["); P(i); P("]=");P(_formats[i]);
  }
  PL("");
}

void Config::printJson() const {
  StaticJsonDocument<JSON_DOC_SIZE> doc;
  saveToJson(doc);
  serializeJson(doc,Serial); Serial.println();
}

void Config::saveToJson(JsonDocument& doc) const {
  // Set the values in the document

  doc["mode"]     = _mode;
  JsonArray formats = doc.createNestedArray("formats");
  for (int i=0;i < N_MODES; i++) {
    formats.add(_formats[i]);
  }
  JsonObject msgs = doc.createNestedObject("msgs");
  msgs["start"] = _msgStart.c_str();
  msgs["end"]   = _msgEnd.c_str();

  JsonObject times = doc.createNestedObject("times");
  times["start"] = _timeStart.c_str();
  times["end"]   = _timeEnd.c_str();

  JsonObject network = doc.createNestedObject("network");
  network["ssid"]     = _apSSID.c_str();
  network["password"] = _apPassword.c_str();
  /*
  P("Json memory usage: "); 
  PV(doc.memoryUsage()); SPACE;
  PV(doc.capacity()); SPACE;
  PV(doc.overflowed()); SPACE;
  PL("");
  */
}

// Saves the configuration to a file
void Config::saveFile(void) const {
  const char* filename = getFileName().c_str();

  // Delete existing file, otherwise the configuration is appended to the file
  PL("trying to save "); PL(filename);
  StaticJsonDocument<JSON_DOC_SIZE> doc;
  saveToJson(doc);
  serializeJsonPretty(doc,Serial); PL("");

  if (FILESYSTEM.exists(filename)) 
    FILESYSTEM.remove(filename);

  // Open file for writing
  File file = FILESYSTEM.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.

 // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();

  bool exists = FILESYSTEM.exists(filename);
  P("success? "); PL(exists);
  PVL(filename);
  if (exists) {
      file = FILESYSTEM.open(filename, FILE_READ);
      size_t filesize = file.size();
      file.close();
      PVL(filesize);
  }
}
void Config::loadFromJson(const JsonDocument& doc) {
  char buffer[32];
  // Set the values in the document
  _mode = doc["mode"];

  for (int i=0;i < N_MODES; i++) {
    _formats[i] = doc["formats"][i];
  }
  strlcpy(buffer, doc["msgs"]["start"] | _msgStart.c_str(), CHARS_PER_MESSAGE+1); 
  _msgStart = buffer;
  strlcpy(buffer, doc["msgs"]["end"] | _msgEnd.c_str(), CHARS_PER_MESSAGE+1); 
  _msgEnd = buffer;

  strlcpy(buffer, doc["time"]["start"] | _timeStart.c_str(), ISOTIME_SIZE+1); 
  _timeStart = buffer;
  strlcpy(buffer, doc["time"]["end"] | _timeEnd.c_str(), ISOTIME_SIZE+1); 
  _timeEnd = buffer;
  
  strlcpy(buffer, doc["network"]["ssid"] | _apSSID.c_str(), SSID_SIZE+1); 
  _apSSID = buffer;
  strlcpy(buffer, doc["network"]["password"] | _apPassword.c_str(), SSID_SIZE+1); 
  _apPassword = buffer;
}

// Loads the configuration from a file
bool Config::loadFile(void) {
  // Open file for reading
  const char* filename = getFileName().c_str();
  if (!FILESYSTEM.exists(filename)) {
    P("loadFile: ERROR: file ");P(filename);PL(" not found");
    return false;
  }
  File file = FILESYSTEM.open(filename, FILE_READ);
  if (!file) {
    Serial.println(F("loadFile: Failed to open file ")); PL(filename);
    return false;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<JSON_DOC_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error) 
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  loadFromJson(doc);
  P("loadFile: success loading "); PL(filename);

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
  return true;
}