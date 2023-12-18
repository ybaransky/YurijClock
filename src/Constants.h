#pragma once
#include <Arduino.h>

// RTC 3231   I2C  3.3v - 5v
#define RTC_SCL_PIN D1  // set somewhere in Wire as default
#define RTC_SDA_PIN D2 
#define RTC_SQW_PIN D7

// OneButton Interrups
#define ONEBUTTON_PIN   D8

// TM1637 displays  
#define COMMON_CLK   1
#ifdef COMMON_CLK
    #define TM1637_CLK_0    D3
    #define TM1637_DIO_0    D4

    #define TM1637_CLK_1    D3  // D5
    #define TM1637_DIO_1    D6

    #define TM1637_CLK_2    D3  // RX
    #define TM1637_DIO_2    D6  // TX
#else
    #define TM1637_CLK_0    D3
    #define TM1637_DIO_0    D4

    #define TM1637_CLK_1    D5
    #define TM1637_DIO_1    D6

    #define TM1637_CLK_2    RX
    #define TM1637_DIO_2    TX
#endif

// tm1637 display
#define LEADING_ZERO  true
#define BRIGHTEST  7
#define DIMMEST  1
#define COLON  0x40 // 0b01000000;


enum  {
  SSUU=0,HHMM, DDDD, N_SEGMENTS,
  DIGITS_PER_SEGMENT=4,
  MODE_COUNTDOWN=0, MODE_CLOCK,MODE_MESSAGE,MODE_DEMO, N_MODES
};



