#pragma once

enum  {
  SSUU=0,HHMM, DDDD, N_SEGMENTS,
  DIGITS_PER_SEGMENT=4,
  CHARS_PER_MESSAGE=12,
  MODE_COUNTDOWN=0, MODE_COUNTUP, MODE_CLOCK, MODE_TEXT, N_MODES,
  FORMAT_COUNTDOWN=8, FORMAT_COUNTUP=8, FORMAT_CLOCK=13, FORMAT_TEXT=2,
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