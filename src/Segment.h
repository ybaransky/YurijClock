#pragma once
#include <Arduino.h>
#include <TM1637Display.h>
typedef TM1637Display Device;
 
struct Digits {
    int d1000;
    int d100;
    int d10;
    int d1;
    char c1000;
    char c100;
    char c10;
    char c1;
    Digits(int value=0);
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
        void    init(int iam);
        Device& device(void);

        void    drawDDDD(int);
        void    drawHHMM(int,int);
        void    drawSSUU(int,int,uint8_t);

        bool    changed();
        void    reverse();
        void    saveToCache();
        void    setFormat(int);
        void    setSegment(bool colon=false);

        int         _iam;
        int         _format;
        uint8_t     _data[4]; 
        uint8_t     _cache[4]; 
};
