#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include <TM1637Display.h>

typedef TM1637Display Device;

class Segment {

    struct Data{
      bool      operator==(const Data&);
      Data&     operator=(const Data&);

      void      init(void);
      void      set(uint8_t*, uint8_t, bool);
      void      print(const char* msg=nullptr);

      uint8_t   _buffer[4];
      uint8_t   _brightness;
      bool      _visible;
    };

    public:
        static  uint8_t  encodeChar(char);
        static  uint8_t  encodeDigit(uint8_t);
        static  void     reverse(uint8_t*, int n=4);

        void    init(int iam);
        Device& device(void);

        void    setBrightness(uint8_t);
        void    setVisible(bool);
        void    write(uint8_t data[], bool colon, uint8_t brightness=7, bool visible=true);

    private:
        int     _iam;
        Data    _data,_cache;
};
