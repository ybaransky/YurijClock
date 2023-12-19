#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include <TM1637Display.h>

typedef TM1637Display Device;
 
struct Digits {
    int  d1000; int  d100; int  d10; int  d1; // decimal of the n-th dogot
    char c1000; char c100; char c10; char c1; // character of the n-th digit
    Digits(int value=0);
    Digits(uint8_t value=0);
    void  set(int);
};

class Segment {

    struct Data{
      bool    operator==(const Data&);
      Data&   operator=(const Data&);

      void    set(uint8_t*, uint8_t, bool);
      void    init();
      void    reverse(void);
      void    addColon(bool);

      uint8_t _buffer[4];
      uint8_t _brightness;
      bool    _visible;
    };

    public:
        static  uint8_t  encodeChar(char);
        static  uint8_t  encodeDigit(uint8_t);
        static  char*    reverse(char*);
        static  uint8_t* reverse(uint8_t*);

        void    init(int iam);
        Device& device(void);

        void    drawDDDD(const TimeSpan&, int);
        void    drawHHMM(const TimeSpan&, int);
        void    drawSSUU(const TimeSpan&, uint8_t, int);

        void    drawDDDD(const DateTime&, int);
        void    drawHHMM(const DateTime&, int);
        void    drawSSUU(const DateTime&, uint8_t, int);

        void    drawText(char* text, bool visible);

        void    setBrightness(uint8_t);
        void    setVisible(bool);
        void    write(uint8_t data[], bool colon, uint8_t brightness=7, bool visible=true);

    private:
        void    setSegment(bool colon=false,bool print=false);
        void    encode(char, char, char, char);
        void    reverse(void);

        int     _iam;
        Data    _data,_cache;
};
