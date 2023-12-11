#pragma once
#include <Arduino.h>
#include <TM1637Display.h>
typedef TM1637Display Device;
 
struct Digits {
    int d1000;
    int d100;
    int d10;
    int d1;
    Digits(int value=0);
    void  set(int);
};

class Segment {

    static Device   devices[3];
    static int      format;
    static uint8_t  asciEncoding[96];

    void     reverse(uint8_t*);
    uint8_t  encodeChar(char);
    uint8_t  encodeDigit(uint8_t);

    public:
        void    init(int iam);
        Device& device(void);

        void    drawDDDD(int);
        void    drawHHMM(int,int);
        void    drawSSUU(int,uint8_t);
        void    drawSSUU(int);

        bool    changed(uint8_t*);
        void    saveToCache(uint8_t*);
        void    setSegment(uint8_t* data,bool colon=false);

        int     _iam;
        byte    _data[4]; 
};
