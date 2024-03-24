#include <Arduino.h>
#include "Constants.h"
#include "Config.h"
#include "Debug.h"

#define JSON_DOC_SIZE 512
#define FILE_WRITE  "w"
#define FILE_READ   "r"

static const char*  NAME="Config::";

Config* initConfig(void) {
    Config* cfg = new Config();
    cfg->init();
    if (!cfg->loadFile())  // if file not file, create it
      cfg->saveFile();

    cfg->print();
    cfg->printFile();
    return cfg;
}

void Config::init(void) {
  P(NAME);PL("init");

  FILESYSTEM.begin();

  setTimeStart(DEFAULT_TIME_START);
  setTimeEnd(  DEFAULT_TIME_END);

  setMsgStart(DEFAULT_MESSAGE_START);
  setMsgEnd(  DEFAULT_MESSAGE_END);

  setPassword(DEFAULT_AP_PASSWORD);
  setSSID(    DEFAULT_AP_SSID);

  _mode = DEFAULT_MODE;
  memset(_formats,DEFAULT_FORMAT,sizeof(_formats));

  setHourMode(DEFAULT_HOUR_MODE);
  setSecsMode(DEFAULT_SECS_MODE);
  setBrightness(DEFAULT_BRIGHTNESS);
}

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
    default: return 0;
  }
}
void  Config::setFormat(int format) { 
  P(NAME); P("setFormat"); P(" changing format ");
  P(_formats[_mode]); P(" --> "); PL(format);
  _formats[_mode] = format;
}
void  Config::setFormat(int format, int mode) { 
  P(NAME); P("setFormat(format,mode)"); P(" changing format ");
  P(_formats[mode]); P(" --> "); PL(format);
  _formats[mode] = format;
}

int   Config::getHourMode(void) { return _hourMode;}
void  Config::setHourMode(int hourMode) { _hourMode = hourMode;}

int   Config::getSecsMode(void) { return _secsMode;}
void  Config::setSecsMode(int secsMode) { _secsMode = secsMode;}

bool  Config::isTenthSecFormat(void) {
  bool rc;
  int format = _formats[_mode];
  switch(_mode) {
    case MODE_COUNTUP : 
    case MODE_COUNTDOWN : return (format==0) || (format==4); 
    case MODE_CLOCK :     return (format==3) || (format==7);
    default : return false;
  }
  return rc;
}

const String& Config::getSSID(void) { return _apSSID;}
void          Config::setSSID(const String& ssid, const char* fcn) {
  if (fcn) {
    P(fcn);P("setSSID: "); P(_apSSID);P("-->");PL(ssid);
  }
  _apSSID = ssid;
} 
const String& Config::getPassword(void) { return _apPassword;}
void          Config::setPassword(const String& password, const char* fcn) {
  if (fcn) {
    P(fcn);P("setPassword: "); P(_apPassword);P("-->");PL(password);
  }
  _apPassword = password;
} 


uint8_t Config::getBrightness(void) { return _brightness;}
void    Config::setBrightness(uint8_t brightness) { _brightness = brightness; }

static bool changedString(const char* desc, const String& before, const String& after) {
  if (before != after) {
    P("changing "); P(desc); SPACE; P(before); P(" --> "); PL(after);
    return true;
  }
  return false;
}

const DateTime& Config::getTimeStartDT(void) { return _timeStartDT;}
const String&   Config::getTimeStart(void) { return _timeStart;}
void            Config::setTimeStart(const String& time) { 
  if (changedString("timeStart", _timeStart, time)) {
    _timeStart = time;
    _timeStartDT = DateTime(_timeStart.c_str());
  }
}
const DateTime& Config::getTimeEndDT(void) { return _timeEndDT;}
const String&   Config::getTimeEnd(void) { return _timeEnd;}
void            Config::setTimeEnd(const String& time) {
  if (changedString("timeEnd", _timeEnd, time)) {
    _timeEnd = time;
    _timeEndDT = DateTime(_timeEnd.c_str());
  }
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

void Config::print(void) const {
  StaticJsonDocument<JSON_DOC_SIZE> doc;
  configToJson(doc);
  serializeJson(doc,Serial); 
  Serial.println();
}

void Config::printFile(void) const {
  String str;
  fileToString(str);
  Serial.println(str);
}



///////////////////////////////////////////////////////////////////////////////////

void Config::configToJson(JsonDocument& doc) const {
  // Set the values in the document
  doc["mode"]       = _mode;
  doc["hourMode"]   = _hourMode;
  doc["secsMode"]   = _secsMode;
  doc["brightness"] = _brightness;
  doc["msgStart"]   = _msgStart.c_str();
  doc["msgEnd"]     = _msgEnd.c_str();
  doc["timeStart"]  = _timeStart.c_str();
  doc["timeEnd"]    = _timeEnd.c_str();
  doc["ssid"]       = _apSSID.c_str();
  doc["password"]   = _apPassword.c_str();
  JsonArray formats = doc.createNestedArray("formats");
  for (int i=0;i < N_MODES; i++) { 
    formats.add(_formats[i]); 
  }
}

void Config::jsonToConfig(const JsonDocument& doc) {
  char buffer[32];
  _mode       = doc["mode"]        | DEFAULT_MODE;
  _hourMode   = doc["hourMode"]    | DEFAULT_HOUR_MODE;
  _secsMode   = doc["secsMode"]    | DEFAULT_SECS_MODE;
  _brightness = doc["brightness"]  | DEFAULT_BRIGHTNESS;
  strlcpy(buffer, doc["msgStart"]  | DEFAULT_MESSAGE_START, MESSAGE_SIZE+1); setMsgStart(buffer);
  strlcpy(buffer, doc["msgEnd"]    | DEFAULT_MESSAGE_END,   MESSAGE_SIZE+1); setMsgEnd(buffer);
  strlcpy(buffer, doc["timeStart"] | DEFAULT_TIME_START,    ISOTIME_SIZE+1); setTimeStart(buffer);
  strlcpy(buffer, doc["timeEnd"]   | DEFAULT_TIME_END,      ISOTIME_SIZE+1); setTimeEnd(buffer);
  strlcpy(buffer, doc["ssid"]      | DEFAULT_AP_SSID,       SSID_SIZE+1);    setSSID(buffer);
  strlcpy(buffer, doc["password"]  | DEFAULT_AP_PASSWORD,   SSID_SIZE+1);    setPassword(buffer);
  for (int i=0;i < N_MODES; i++) {
    _formats[i] = doc["formats"][i] | DEFAULT_FORMAT;
  }
}

// Loads the configuration from a file
bool Config::fileToString(String& jsonStr) const {
  StaticJsonDocument<JSON_DOC_SIZE> doc;
  File file = getFileObject(FILE_READ);
  if (!fileToJson(file,doc))
    return false;
  serializeJsonPretty(doc,jsonStr);
  return true;
}

// Loads the configuration from a file
bool Config::loadFile(void) {
  P(NAME);PL("loadFile");
  StaticJsonDocument<JSON_DOC_SIZE> doc;
  File file = getFileObject(FILE_READ);
  bool fileFound = fileToJson(file,doc);
  jsonToConfig(doc);
  if (!fileFound) {
    P("file "); P(CONFIG_FILENAME); PL(" not found, loading default");
  }
  file.close();
  return fileFound;
}

bool  Config::fileToJson(File& file, JsonDocument& doc) const {
  if (file) {
    DeserializationError error = deserializeJson(doc, file);
    printJsonMemoryStats(doc);
    if (error) {
      P(F("Failed to deserialzie json file ")); PL(CONFIG_FILENAME);
      return false;
    }
  }
  return true;
}

bool    Config::saveFile(const char* fcn) const {
  bool success = false;
  File file = getFileObject(FILE_WRITE);
  if (!file) {
    Serial.print(F("Failed to create file"));
    Serial.println(CONFIG_FILENAME);
    file.close();
    return success;
  }
  StaticJsonDocument<JSON_DOC_SIZE> doc;
  configToJson(doc);
  serializeJsonPretty(doc,Serial); PL("");
  if (!jsonToFile(doc, file)) {
    Serial.println(F("Failed to write to file"));
  } else {
    size_t filesize = file.size();
    PV(filename);SPACE;PVL(filesize);
    success = true;
  }
  // Close the file
  file.close();
  return success;
}

bool  Config::jsonToFile(JsonDocument& doc, File& file) const {
  return (serializeJson(doc, file) != 0);
}

File Config::getFileObject(const char* mode) const {
  const char* filename = CONFIG_FILENAME;
  File file = FILESYSTEM.open(filename,mode);
  if (!file) {
    if (!strcmp(mode,FILE_READ)) {
        P(F("loadFile: Failed to open file "));
    } else {
        P(F("loadFile: Failed to create file "));
    } 
    PL(filename);
  }
  return file;
}

void Config::printJsonMemoryStats(JsonDocument& doc) const {
  P("Json memory usage: "); 
  PV(doc.memoryUsage()); SPACE;
  PV(doc.capacity()); SPACE;
  PV(doc.overflowed()); SPACE;
  PL("");
}