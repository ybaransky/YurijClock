#include <Arduino.h>
#include "Display.h"
#include "Config.h"
#include "Constants.h"
#include "Digits.h"
#include "Debug.h"
#include "Test.h"
/*
12/7/2023
For some reason, I can't create the TM1637Display objects via new. 
This needs to be understood 
*/

#define SEGMENT_BRIGHTEST 7
#define SEGMENT_DIMMEST   1
#define SEGMENT_ON        true
#define SEGMENT_OFF       false
#define SEGMENT_COLON     0x40

static char    space = ' '; 
static char    dchar = 'd';
static char    hchar = 'h';
static char    nchar = 'n';


/*
*************************************************************************
*  Display Class
*************************************************************************
*/

Display* initDisplay(void) {
  Display* display = new Display();
  display->init();
  return display;
}

void Display::init(void) {
  for(int i=0; i < N_SEGMENTS; i++) 
    _segments[i].init(i);
  clear();
}

void Display::clear(void) {
  const char* fcn = "Display:clear:";
  uint8_t brightness = config->_brightness;
  PL(fcn);
  for(auto& segment : _segments) {
    segment.device().clear();
    segment.setBrightness(brightness);
    segment.setVisible(true);
  }
}

void Display::test(void) {
  int values[] = {0000,1111,2222};
  bool zeroPad = true;
  bool on=true;
  // testDevice(_segments[0].device(),500);

  for(int k=0; k < 4; k++) {
    for(int i=0;i<N_SEGMENTS;i++) {
      P(i);PVL(values[i]);
      _segments[i].device().setBrightness(SEGMENT_BRIGHTEST,on);
      _segments[i].device().showNumberDecEx(values[i], SEGMENT_COLON, zeroPad);
    }
    delay(250);
    on = !on;
  }
  for(auto& segment : _segments)
    segment.device().setBrightness(SEGMENT_BRIGHTEST,true);
}

void Display::showInteger(int32_t ival) {
    int parts[3];
    parts[2] =  ival / 100000000;
    parts[1] = (ival % 100000000) / 10000;
    parts[0] = (ival % 10000);
    for(int i=0; i<3; i++)
      _segments[i].device().showNumberDec(parts[i],false);
}

void  Display::refresh(const char* caller) {
  if (caller) {
    P("display::refresh (doing nothing) called from ");PL(caller);
  } else {
    PL("display::refresh (doing nothing)" );
  }
}

/*
******************************************************************************************
****************** Public ****************************************************************
******************************************************************************************
*/

void Display::showCount(const TimeSpan& ts, uint8_t tenth) {
  int format = config->getFormat();
  showCountDDDD(ts,format);
  showCountHHMM(ts,format);
  showCountSSUU(ts,tenth,format);

}
void Display::showClock(const DateTime& dt, uint8_t tenth) {
  int format = config->getFormat();
  showClockDDDD(dt,format);
  showClockHHMM(dt,format);
  showClockSSUU(dt,tenth,format);
}

/*
  we have 3 segements, left to right, they are numbered
  0123 0123 0123    // left most is most significant, indexed by 0
  seg2 seg1 seg0
  each segements 4 digits, they are addressed 0123
  we want to test buffer to be "right justified"
  lets say the test is
   01234567890a  12 chars
  "abcdefghijkl"
  test[0-3] --> seg2 (abcd)
  test[4-7] --> seg1 (efgh)
  test[8-a] --> seg0 (ijkl)

  
*/
void Display::showText(const String& msg, bool visible) {
  char     buffer[13];
  uint8_t  data[12];
  bool     colon = false;
  snprintf(buffer,13,"%-12s",msg.c_str());
  // encode the buffer the entire buffer
  for(int i=0;i<MESSAGE_SIZE;i++)
    data[i] = Segment::encodeChar(buffer[i]);
  // reverse it
  Segment::reverse(data,MESSAGE_SIZE);
  // write it
  writeSegment(SSUU, &data[0], colon, visible);
  writeSegment(HHMM, &data[4], colon, visible);
  writeSegment(DDDD, &data[8], colon, visible);
}

/*
******************************************************************************************
****************** Private ****************************************************************
******************************************************************************************
*/

void  Display::showCountDDDD(const TimeSpan& ts, int format) {
  Digits  days(ts.days());
  uint8_t data[DIGITS_PER_SEGMENT];
  bool    showD = (format < 4);

  if (days.d1000) {
    encode(data, days.c1000, days.c100, days.c10, days.c1);
  } else {
    if (showD) {
      if (days.d100)     encode(data, days.c100, days.c10, days.c1, dchar);
      else if (days.d10) encode(data,  days.c10,  days.c1,   space, dchar);
      else if (days.d1)  encode(data,     space,  days.c1,   space, dchar);
      else               encode(data,     space,    space,   space, space); 
    } else {
      if (days.d100)     encode(data, space, days.c100, days.c10, days.c1);
      else if (days.d10) encode(data, space,     space, days.c10, days.c1);
      else if (days.d1)  encode(data, space,     space,    space, days.c1); 
      else               encode(data, space,     space,    space,   space); 
    }
  }
  writeSegment(DDDD,data);
}

void  Display::showCountHHMM(const TimeSpan& ts, int format) {
  uint8_t data[DIGITS_PER_SEGMENT];
  Digits  hours(ts.hours());
  Digits  mins(ts.minutes());
  bool    showHoursH    = (format==2) || (format==3);
  bool    showHours     = (format==6) || (format==7);
  bool    showHoursMins = (format==0) || (format==1) || (format==4) || (format==5);
  bool    colon         = showHoursMins;

  if ((ts.hours()==0) && (ts.minutes()==0)) {
     encode(data,     space, space, space, space);
     colon = false;
  } else {
  	if (showHoursMins) {
      if (hours.d10)  encode(data, hours.c10, hours.c1, mins.c10, mins.c1);
      else            encode(data,     space, hours.c1, mins.c10, mins.c1);
    } else if (showHoursH) {
      if (hours.d10) encode(data, hours.c10, hours.c1, space, hchar);
      else           encode(data,     space, hours.c1, space, hchar);
    } else if (showHours) {
      if (hours.d10) encode(data, space, space, hours.c10, hours.c1);
      else           encode(data, space, space,     space, hours.c1);
    }
  }
  writeSegment(HHMM,data,colon);
};

void  Display::showCountSSUU(const TimeSpan& ts, uint8_t ms100, int format) {
  uint8_t data[DIGITS_PER_SEGMENT];
  Digits  mins(ts.minutes());
  Digits  secs(ts.seconds());
  Digits  ms(ms100);
  bool    showMillis   = (format==0) || (format==4);
  bool    showSecs     = (format==1) || (format==5);
  bool    showMinsN    = (format==3);
  bool    showMins     = (format==7);
  bool    showMinsSecs = (format==2) || (format==6);
  bool    colon        = showMinsSecs;

  if (showMillis) {
    if (secs.d10) encode(data, secs.c10, secs.c1, space, ms.c1);
    else          encode(data, space,    secs.c1, space, ms.c1);
  } else if (showSecs) {
    if (secs.d10) encode(data, space, space, secs.c10, secs.c1);
    else          encode(data, space, space, space,    secs.c1);
  } else if (showMinsN) {
    if (mins.d10) encode(data, mins.c10, mins.c1, space, nchar);
    else          encode(data, space,    mins.c1, space, nchar);
  } else if (showMins) {
    if (mins.d10) encode(data, space, space, mins.c10, mins.c1);
    else          encode(data, space, space, space,    mins.c1);
  } else {
    if (mins.d10) encode(data, mins.c10, mins.c1, secs.c10, secs.c1);
    else          encode(data, space,    mins.c1, secs.c10, secs.c1);
  }
  writeSegment(SSUU,data,colon);
};

/*
*********************Clock ***************************************************************
*/
void  Display::showClockDDDD(const DateTime& dt, int format) {
  uint8_t data[DIGITS_PER_SEGMENT];
  Digits  years(dt.year());
  Digits  mons(dt.month());
  Digits  days(dt.day());
  bool    showYears    = (format==0) || (format==1) || (format==2);
  bool    showMons     = (format==3) || (format==4);
  bool    showDays     = (format==9) || (format==10) || (format==11) || (format==12);
  bool    showMonsDays = (format==5) || (format==6) || (format==7) || (format==8);
  bool    colon        = showMonsDays;

  if (showYears) {
    encode(data, years.c1000, years.c100, years.c10, years.c1);
  } else if (showMons) {
    if (mons.d10)   encode(data, space, space, mons.c10, mons.c1);
      else          encode(data, space, space,    space, mons.c1);
  } else if (showDays) {
    if (days.d10)   encode(data, space, space, days.c10, days.c1);
      else          encode(data, space, space,    space, days.c1);
    } else {
      if (mons.d10) encode(data, mons.c10, mons.c1, days.c10, days.c1);
      else          encode(data,    space, mons.c1, days.c10, days.c1);
    }
    writeSegment(DDDD, data, colon);
};

void  Display::showClockHHMM(const DateTime& dt, int format)  {
  uint8_t data[DIGITS_PER_SEGMENT];
  Digits  mons(dt.month());
  Digits  days(dt.day());
  Digits  hours(dt.hour());
  Digits  mins(dt.minute());
  bool    showMonsDays  =  (format==0) || (format==1);
  bool    showMons      =  (format==2);
  bool    showDays      =  (format==3) || (format==4);
  bool    showHoursMins =  (format==5) || (format==6) || (format==9) || (format==10);
  bool    showHours     =  (format==7) || (format==8) || (format==11) || (format==12);
  bool    colon         = showMonsDays || showHoursMins;

  if (showMonsDays) {
    if (mons.d10)  encode(data, mons.c10, mons.c1, days.c10, days.c1);
    else           encode(data,    space, mons.c1, days.c10, days.c1);
  } else if (showMons) {
    if (mons.d10)  encode(data, space, space, mons.c10, mons.c1);
    else           encode(data, space, space,    space, mons.c1);
  } else if (showDays) {
    if (days.d10)  encode(data, space, space, days.c10, days.c1);
    else           encode(data, space, space,    space, days.c1);
  } else if (showHoursMins) {
    if (hours.d10) encode(data, hours.c10, hours.c1, mins.c10, mins.c1);
    else           encode(data,     space, hours.c1, mins.c10, mins.c1);
  } else if (showHours) {
    if (hours.d10) encode(data, space, space, hours.c10, hours.c1);
    else           encode(data, space, space,     space, hours.c1);
  }
  writeSegment(HHMM, data, colon);
};
 
void  Display::showClockSSUU(const DateTime& dt, uint8_t ms100,int format) {
  uint8_t data[DIGITS_PER_SEGMENT];
  Digits  days(dt.day());
  Digits  hours(dt.hour());
  Digits  mins(dt.minute());
  Digits  secs(dt.second());
  Digits  ms(ms100);
  bool    showHoursMins = (format==0) || (format==1) || (format==3) || (format==4);
  bool    showDays      = (format==2);
  bool    showMillis    = (format==5) || (format==9);
  bool    showSecs      = (format==6) || (format==10);
  bool    showMinsSecs  = (format==7) || (format==11);
  bool    showMins      = (format==12);
  bool    blinking      = (format==0) || (format==3);
  bool    colon         = showHoursMins || showMinsSecs;
  if (blinking) {
    colon = dt.second() % 2;
  }

  if (showHoursMins) {
    if (hours.d10) encode(data, hours.c10, hours.c1, mins.c10, mins.c1);
    else           encode(data,     space, hours.c1, mins.c10, mins.c1);
  } else if (showDays) {
    if (days.d10)  encode(data, space, space, days.c10, days.c1);
    else           encode(data, space, space,    space, days.c1);
  } else if (showMillis) {
    if (secs.d10)  encode(data, secs.c10, secs.c1, space, ms.c1);
    else           encode(data,    space, secs.c1, space, ms.c1);
  } else if (showSecs) {
    if (secs.d10)  encode(data, space, space, secs.c10, secs.c1);
    else           encode(data, space, space,    space, secs.c1);
  } else if (showMinsSecs) {
    if (mins.d10)  encode(data, mins.c10, mins.c1, secs.c10, secs.c1);
    else           encode(data,    space, mins.c1, secs.c10, secs.c1);
  } else if (showMins) {
    if (mins.d10)  encode(data, space, space, mins.c10, mins.c1);
    else           encode(data, space, space,    space, mins.c1);
  }
  writeSegment(SSUU,data,colon);
};


/*
*********************Clock ***************************************************************
*/
void  Display::encode(uint8_t data[],char c3, char c2, char c1, char c0) {
  data[3] = Segment::encodeChar(c3);
  data[2] = Segment::encodeChar(c2);
  data[1] = Segment::encodeChar(c1);
  data[0] = Segment::encodeChar(c0);
}

void  Display::writeSegment(int id, uint8_t data[], bool colon, bool visible) {
  uint8_t brightness = config->getBrightness();
  _segments[id].write(data, colon, brightness, visible);
}