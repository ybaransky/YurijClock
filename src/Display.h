#pragma once
#include <RTCLib.h>
#include <TM1637Display.h>

typedef TM1637Display Device;

#define SEGMENT_BRIGHTEST 7
#define SEGMENT_DIMMEST   1
#define SEGMENT_ON        true
#define SEGMENT_OFF       false
#define SEGMENT_COLON     0x40

#define SSUU  0  
#define HHMM  1
#define YYMM  2

class Display {
  
  static const int N_SEGMENTS = 3;
  public:
    void    init(void);
    void    test(void);

    void    enable(bool);
    void    setBrightness(uint8_t brightness,bool on=true);
    void    showInteger(int32_t);
    void    showTimeSpan(TimeSpan&);

  private:
    Device& getDevice(int i);
    char    _message[13];
};

extern Display* initDisplay(void);