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
<<<<<<< HEAD
  MODE_COUNTDOWN=0, MODE_COUNTUP, MODE_CLOCK, N_MODES,
  N_FORMAT_COUNTDOWN=8, N_FORMAT_COUNTUP=8, N_FORMAT_CLOCK=11,
  HOUR_MODE_24=0,     HOUR_MODE_12=1,      N_HOUR_MODES,
  SECS_MODE_BLINK=0,  SECS_MODE_NOBLINK=1, N_SECS_MODES,
=======
  MODE_COUNTDOWN=0, MODE_COUNTUP, MODE_CLOCK, MODE_TEXT, MODE_DEMO, N_MODES,
  N_FORMAT_COUNTDOWN=8, N_FORMAT_COUNTUP=8, N_FORMAT_CLOCK=13, N_FORMAT_TEXT=1, N_FORMAT_DEMO=1,
  N_HOUR_FORMATS=2,
  HOUR_FORMAT_24=0, HOUR_FORMAT_12=1,
>>>>>>> 727ced071f3559396ad7d1b6e8b82d5ad214b861
};

#define N_ITEMS(x)   (sizeof(x) / sizeof((x)[0]))     // number of elements in array x

// tm1637 display
#define LEADING_ZERO  true
#define BRIGHTEST  7
#define DIMMEST  1
#define COLON  0x40 // 0b01000000;


// defaults for the config file
#define CONFIG_FILENAME    "/countdown.json"

#define ISOTIME_SIZE 16
#define SSID_SIZE 16
#define PASSWORD_SIZE 16
//                           1234567890123456
#define DEFAULT_TIME_START  "2023-12-22T14:45"    // no seconds
#define DEFAULT_TIME_END    "2024-07-28T15:45"

#define DEFAULT_MESSAGE_START "YuriCloc"
#define DEFAULT_MESSAGE_END   "Good Luc"

#define DEFAULT_AP_SSID       "YuriClock"
#define DEFAULT_AP_PASSWORD   "12345678"

#define DEFAULT_MODE          MODE_COUNTDOWN
#define DEFAULT_FORMAT        0
#define DEFAULT_BRIGHTNESS    7   //  1 ... 7
<<<<<<< HEAD
#define DEFAULT_HOUR_MODE     HOUR_MODE_24     // 24 hour mode
#define DEFAULT_SECS_MODE     SECS_MODE_BLINK  // blink the seconds
=======
#define DEFAULT_HOUR_FORMAT   HOUR_FORMAT_24   // 24 hour mode
>>>>>>> 727ced071f3559396ad7d1b6e8b82d5ad214b861


extern const char* formatNamesCountDown[];
extern const char* formatNamesCountUp[];
extern const char* formatNamesClock[];

extern const char* modeNames[];

extern const char* hourModeNames[];
extern const int   hourModeValues[];
extern const char* secsModeNames[];
extern const int   secsModeValues[];