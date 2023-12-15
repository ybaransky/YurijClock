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

    struct Data{
      bool    operator==(const Data&);
      Data&   operator=(const Data&);

      void    init(void);
      void    reverse(void);
   //   Data    operator+(const Data& data);

      uint8_t _buffer[4];
      uint8_t _brightness;
      bool    _visible;
    };

    static Device   devices[3];

    public:
//      static  uint8_t* reverse(uint8_t*);
//      static  char*    reverse(char*);

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

    private:
        void    setSegment(bool colon=false,bool print=false);
        void    encode(char, char, char, char);

        int     _iam;
        Data    _data;
        Data    _cache;
};
