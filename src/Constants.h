#pragma once
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>

typedef ESP8266WebServer WebServer;

enum  {
  SSUU=0,HHMM, DDDD, N_SEGMENTS,
  DIGITS_PER_SEGMENT=4,
  CHARS_PER_MESSAGE=12,
  MODE_COUNTDOWN=0, MODE_COUNTUP, MODE_CLOCK, MODE_TEXT, N_MODES,
  N_FORMAT_COUNTDOWN=8, N_FORMAT_COUNTUP=8, N_FORMAT_CLOCK=13, N_FORMAT_TEXT=2,
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

extern const char* modeNames[];