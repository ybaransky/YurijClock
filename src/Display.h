#pragma once
#include <RTClib.h>
#include "Constants.h"
#include "Segment.h"

class Display {
  public:
    void    init(void);
    void    clear(void);

    void    test(void);

    void    showInteger(int32_t);
//    void    setBrightness();
    void    refresh(const char* caller=nullptr);

    void    showCount(const TimeSpan&, uint8_t tenth=0, int format=0);
    void    showClock(const DateTime&, uint8_t tenth=0, int format=0);
    void    showText(const String&, bool visible);

  private:
    void    showCountDDDD(const TimeSpan&, int format);
    void    showCountHHMM(const TimeSpan&, int format);
    void    showCountSSUU(const TimeSpan&, uint8_t tenth, int format);
    
    void    showClockDDDD(const DateTime&, int);
    void    showClockHHMM(const DateTime&, int format, int hourMode);
    void    showClockSSUU(const DateTime&, uint8_t tenth, int format, int hourMode, int secsMode);

    void    encode(uint8_t*,char, char, char, char);
    void    writeSegment(int,uint8_t*, bool colon=false,bool visible=true); 

    Segment   _segments[N_SEGMENTS];
};

extern Display* initDisplay(void);