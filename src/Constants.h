#pragma once

#define USE_LITTLEFS
#ifdef USE_LITTLEFS
#define FILESYSTEM  LittleFS
#include <LittleFS.h>
#else
#define FILESYSTEM  SPIFFS
#include <FS.h>>
#endif

//#define USE_ASYNC_WEBSERVER
#ifdef USE_ASYNC_WEBSERVER
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
typedef AsyncWebServer WebServer;
#else 
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>
typedef ESP8266WebServer WebServer;
#endif


enum  {
  SSUU=0,HHMM, DDDD, N_SEGMENTS,
  DIGITS_PER_SEGMENT=4,
  MESSAGE_SIZE=12,
  MODE_COUNTDOWN=0, MODE_COUNTUP, MODE_CLOCK, MODE_TEXT, MODE_DEMO, N_MODES,
  N_FORMAT_COUNTDOWN=8, N_FORMAT_COUNTUP=8, N_FORMAT_CLOCK=13, N_FORMAT_TEXT=1, N_FORMAT_DEMO=1,
};

#define N_ITEMS(x)   (sizeof(x) / sizeof((x)[0]))     // number of elements in array x

// tm1637 display
#define LEADING_ZERO  true
#define BRIGHTEST  7
#define DIMMEST  1
#define COLON  0x40 // 0b01000000;

extern const char* formatNamesCountDown[];
extern const char* formatNamesCountUp[];
extern const char* formatNamesClock[];
extern const char* formatNamesText[];
extern const char* formatNamesDemo[];

extern const char* modeNames[];