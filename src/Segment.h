#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include <TM1637Display.h>

typedef TM1637Display Device;
 
struct Digits {
    int  d1000; int  d100; int  d10; int  d1;
    char c1000; char c100; char c10; char c1;
    Digits(int value=0);
    Digits(uint8_t value=0);
    void  set(int);
};

class Segment {

    static Device   devices[3];
    static uint8_t  asciEncoding[96];
    static char     space;
    static char     dchar;
    static char     hchar;
    static char     nchar;

    void     encode(char, char, char, char);
    uint8_t  encodeChar(char);
    uint8_t  encodeDigit(uint8_t);

    public:
        void    init(int iam, int* _format);
        Device& device(void);

        void    drawDDDD(TimeSpan);
        void    drawHHMM(TimeSpan);
        void    drawSSUU(TimeSpan,uint8_t);

        void    drawDDDD(DateTime);
        void    drawHHMM(DateTime);
        void    drawSSUU(DateTime,uint8_t);

        bool    changed();
        void    reverse();
        void    saveToCache();
        void    setFormat(int,int);
        void    setSegment(bool colon=false);

        int         _iam;
        int         _formats[N_DISPLAY_MODES];
        uint8_t     _data[4]; 
        uint8_t     _cache[4]; 
};
