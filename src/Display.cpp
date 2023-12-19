#include <Arduino.h>
#include "Display.h"
#include "Config.h"
#include "Constants.h"
#include "Debug.h"
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
  uint8_t brightness = config->_brightness;
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
  for(int k=0; k < 2; k++) {
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

/*
******************************************************************************************
*/

void Display::showCount(const TimeSpan& ts, uint8_t tenth) {
  _format = config->getFormat();
  showCountDDDD(ts);
  showCountHHMM(ts);
  showCountSSUU(ts,tenth);
}

void  Display::showCountDDDD(const TimeSpan& ts) {
    Digits  days(ts.days());
    bool    showD = (_format < 4);

    if (days.d1000) {
        encode(days.c1000, days.c100, days.c10, days.c1);
    } else {
        if (showD) {
            if (days.d100)     encode(days.c100, days.c10, days.c1, dchar);
            else if (days.d10) encode( days.c10,  days.c1,   space, dchar);
            else               encode(    space,  days.c1,   space, dchar); 
        } else {
            if (days.d100)     encode(space, days.c100, days.c10, days.c1);
            else if (days.d10) encode(space,     space, days.c10, days.c1);
            else               encode(space,     space,    space, days.c1); 
        }
    }
    writeSegment(DDDD);
}

void  Display::showCountHHMM(const TimeSpan& ts) {
    Digits  hours(ts.hours());
    Digits  mins(ts.minutes());
    int     format        = _format;
    bool    showHoursH    = (format==2) || (format==3);
    bool    showHours     = (format==6) || (format==7);
    bool    showHoursMins = (format==0) || (format==1) || (format==4) || (format==5);
    bool    colon = showHoursMins;

	if (showHoursMins) {
        if (hours.d10) encode(hours.c10, hours.c1, mins.c10, mins.c1);
        else           encode(    space, hours.c1, mins.c10, mins.c1);
    } else if (showHoursH) {
        if (hours.d10) encode(hours.c10, hours.c1, space, hchar);
        else           encode(    space, hours.c1, space, hchar);
    } else if (showHours) {
        if (hours.d10) encode(space, space, hours.c10, hours.c1);
        else           encode(space, space,     space, hours.c1);
    }
    writeSegment(HHMM,colon);
};

void  Display::showCountSSUU(const TimeSpan& ts, uint8_t ms100) {
    Digits  mins(ts.minutes());
    Digits  secs(ts.seconds());
    Digits  ms(ms100);
    int     format = _format;
    bool    showMillis   = (format==0) || (format==4);
    bool    showSecs     = (format==1) || (format==5);
    bool    showMinsN    = (format==3);
    bool    showMins     = (format==7);
    bool    showMinsSecs = (format==2) || (format==6);
    bool    colon = showMinsSecs;

    if (showMillis) {
        if (secs.d10) encode(secs.c10, secs.c1, space, ms.c1);
        else          encode(space,    secs.c1, space, ms.c1);
    } else if (showSecs) {
        if (secs.d10) encode(space, space, secs.c10, secs.c1);
        else          encode(space, space, space,    secs.c1);
    } else if (showMinsN) {
        if (mins.d10) encode(mins.c10, mins.c1, space, nchar);
        else          encode(space,    mins.c1, space, nchar);
    } else if (showMins) {
        if (mins.d10) encode(space, space, mins.c10, mins.c1);
        else          encode(space, space, space,    mins.c1);
    } else {
        if (mins.d10) encode(mins.c10, mins.c1, secs.c10, secs.c1);
        else          encode(space,    mins.c1, secs.c10, secs.c1);
    }
    writeSegment(SSUU,colon);
};

/*
*********************Clock ***************************************************************
*/

void Display::showClock(const DateTime& dt, uint8_t tenth) {
  _format = config->getFormat();
  showClockDDDD(dt);
  showClockHHMM(dt);
  showClockSSUU(dt,tenth);
}

void  Display::showClockDDDD(const DateTime& dt) {
  Digits  years(dt.year());
  Digits  mons(dt.month());
  Digits  days(dt.day());
  int     format = config->getFormat();
  bool    showYears    = (format==0) || (format==1) || (format==2);
  bool    showMons     = (format==3) || (format==4);
  bool    showDays     = (format==9) || (format==10) || (format==11) || (format==12);
  bool    showMonsDays = (format==5) || (format==6) || (format==7) || (format==8);
  bool    colon        = showMonsDays;

  if (showYears) {
    encode(years.c1000, years.c100, years.c10, years.c1);
  } else if (showMons) {
    if (mons.d10) encode(space, space, mons.c10, mons.c1);
      else          encode(space, space,    space, mons.c1);
  } else if (showDays) {
    if (days.d10) encode(space, space, days.c10, days.c1);
      else          encode(space, space,   space, days.c1);
    } else {
      if (mons.d10) encode(mons.c10, mons.c1, days.c10, days.c1);
      else          encode(   space, mons.c1, days.c10, days.c1);
    }
    writeSegment(DDDD,colon);
};

void  Display::showClockHHMM(const DateTime& dt) {
    Digits  mons(dt.month());
    Digits  days(dt.day());
    Digits  hours(dt.hour());
    Digits  mins(dt.minute());
    int     format        = _format;
    bool    showMonsDays  =  (format==0) || (format==1);
    bool    showMons      =  (format==2);
    bool    showDays      =  (format==3) || (format==4);
    bool    showHoursMins =  (format==5) || (format==6) || (format==9) || (format==10);
    bool    showHours     =  (format==7) || (format==8) || (format==11) || (format==12);
    bool    colon         = showMonsDays || showHoursMins;

    if (showMonsDays) {
        if (mons.d10)  encode(mons.c10, mons.c1, days.c10, days.c1);
        else           encode(   space, mons.c1, days.c10, days.c1);
    } else if (showMons) {
        if (mons.d10) encode(space, space, mons.c10, mons.c1);
        else          encode(space, space,    space, mons.c1);
    } else if (showDays) {
        if (days.d10) encode(space, space, days.c10, days.c1);
        else          encode(space, space,    space, days.c1);
    } else if (showHoursMins) {
        if (hours.d10)  encode(hours.c10, hours.c1, mins.c10, mins.c1);
        else            encode(    space, hours.c1, mins.c10, mins.c1);
    } else if (showHours) {
        if (hours.d10) encode(space, space, hours.c10, hours.c1);
        else           encode(space, space,     space, hours.c1);
    }
    writeSegment(HHMM,colon);
};
 
void  Display::showClockSSUU(const DateTime& dt, uint8_t ms100) {
    Digits  days(dt.day());
    Digits  hours(dt.hour());
    Digits  mins(dt.minute());
    Digits  secs(dt.second());
    Digits  ms(ms100);
    int     format        = _format;
    bool    showHoursMins = (format==0) || (format==1) || (format==3) || (format==4);
    bool    showDays      = (format==2);
    bool    showMillis    = (format==5) || (format==9);
    bool    showSecs      = (format==6) || (format==10);
    bool    showMinsSecs  = (format==7) || (format==11);
    bool    showMins      = (format==12);
    bool    colon         = showHoursMins || showMinsSecs;
    bool    blinking      = (format==0) || (format==3);
    if (blinking) {
        colon = dt.second() % 2;
    }

    if (showHoursMins) {
        if (hours.d10) encode(hours.c10, hours.c1, mins.c10, mins.c1);
        else           encode(    space, hours.c1, mins.c10, mins.c1);
    } else if (showDays) {
        if (days.d10) encode(space, space, days.c10, days.c1);
        else          encode(space, space,    space, days.c1);
    } else if (showMillis) {
        if (secs.d10) encode(secs.c10, secs.c1, space, ms.c1);
        else          encode(   space, secs.c1, space, ms.c1);
    } else if (showSecs) {
        if (secs.d10) encode(space, space, secs.c10, secs.c1);
        else          encode(space, space,    space, secs.c1);
    } else if (showMinsSecs) {
        if (mins.d10)  encode(mins.c10, mins.c1, secs.c10, secs.c1);
        else           encode(   space, mins.c1, secs.c10, secs.c1);
    } else if (showMins) {
        if (mins.d10) encode(space, space, mins.c10, mins.c1);
        else          encode(space, space,    space, mins.c1);
    }
    writeSegment(SSUU,colon);
};

/*
*********************Clock ***************************************************************
*/

void Display::showText(const String& msg, bool visible) {
  char buffer[13];
  snprintf(buffer,13,"%-12s",msg.c_str());
  P("Display::showText "); PV(millis()); P("|"); P(buffer); P("|"); PVL(visible);
  
  // reverse this entire buffer
  char tmp[12];
  memcpy(tmp,buffer,sizeof(tmp));
  bzero(buffer,sizeof(buffer));
  for(int i=0;i<12;i++)
    buffer[i] = tmp[11-i];

  memcpy(_data,&buffer[0],sizeof(_data)); writeSegment(SSUU);
  memcpy(_data,&buffer[4],sizeof(_data)); writeSegment(HHMM);
  memcpy(_data,&buffer[8],sizeof(_data)); writeSegment(DDDD);
}

void  Display::encode(char c3, char c2, char c1, char c0) {
  _data[3] = Segment::encodeChar(c3);
  _data[2] = Segment::encodeChar(c2);
  _data[1] = Segment::encodeChar(c1);
  _data[0] = Segment::encodeChar(c0);
}

void  Display::writeSegment(int id, bool colon, bool visible) {
  uint8_t brightness = config->_brightness;

  _segments[id].write(_data, colon, brightness, visible);
}