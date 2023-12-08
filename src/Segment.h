#pragma once
#include <TM1637Display.h>

typedef TM1637Display Device;
class Segment {
  public:
    void    setIndex(int);
    Device& device(void);
    void    test(int timeDelay);

  private:
    union {
      int32_t i;
      byte    bytes[4];
      char    chars[4];
    } u;
    bool      _colon;
    uint8_t   _index;
};