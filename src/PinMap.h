#pragma once

// RTC 3231   I2C  3.3v - 5v
#define RTC_SCL_PIN D1  // set somewhere in Wire as default
#define RTC_SDA_PIN D2 
#define RTC_SQW_PIN D7

// OneButton Interrups
#define ONEBUTTON_PIN   D8

// TM1637 displays  
#define COMMON_CLK   1
#ifdef COMMON_CLK
    #define TM1637_CLK_2    D3 
    #define TM1637_DIO_2    D4  // yyyy

    #define TM1637_CLK_1    D3 
    #define TM1637_DIO_1    D5  // hhmm

    #define TM1637_CLK_0    D3
    #define TM1637_DIO_0    D6  // ssuu


#else
    #define TM1637_CLK_0    D3
    #define TM1637_DIO_0    D4

    #define TM1637_CLK_1    D5
    #define TM1637_DIO_1    D6

    #define TM1637_CLK_2    RX
    #define TM1637_DIO_2    TX
#endif
