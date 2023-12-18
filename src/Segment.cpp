#include <Arduino.h>
#include <RTClib.h>
#include "Constants.h"
#include "Encode.h"
#include "Segment.h"
    
/*
    countdown display modes
    "0 dd D | hh:mm |  ss u",
    "1 dd D | hh:mm |    ss",
    "2 dd D | hh  H | mm:ss",
    "3 dd D | hh  H |  mm n",
    "4   dd | hh:mm |  ss u",
    "5   dd | hh:mm |    ss",
    "6   dd |    hh | mm:ss",
    "7   dd |    hh |    mm"
*/

/* 
    countup display modes
    "0   YYYY | MM:DD | hh:mm",
    "1   YYYY | MM:DD | hh:mm",  // blinking hh:mm colon
    "2   YYYY |    MM |    DD",
    "3     MM |    DD | hh:mm",
    "4     MM |    DD | hh:mm",  // blinking hh:mm colon
    "5  MM:DD | hh:mm | ss  u",
    "6  MM:DD | hh:mm |    ss",
    "7  MM:DD |    hh | mm:ss",
    "8  MM:DD |    hh |    mm",
    "9     DD | hh:mm | ss  u",
    "10    DD | hh:mm |    ss",
    "11    DD |    hh | mm:ss",
    "12    DD |    hh |    mm",
*/
 
/*
*************************************************************************
*  Segment Statics
*************************************************************************
*/

static char    space = ' '; 
static char    dchar = 'd';
static char    hchar = 'h';
static char    nchar = 'n';

#define COMMON_CLK   1
#ifdef COMMON_CLK
static Device  sDevices[N_SEGMENTS] = {Device(D3,D6),Device(D3,D5),Device(D3,D4)};
//static Device  sDevices[N_DEVICES] = {Device(D3,D4),Device(D3,D5),Device(D3,D6)};
#else
static Device sDevices[N_DEVICES] = {Device(D3,D4),Device(D5,D6),Device(RX,TX)};
#endif

/*
Blinking is handled by just looking at the eveness of the seoncds
*/

/*
*************************************************************************
*  Digit Class
*************************************************************************
*/

Digits::Digits(int value) { set(value); }
Digits::Digits(uint8_t value) { set(int(value)); }
void  Digits::set(int value) {
    d1    = value%10; value = value/10;
    d10   = value%10; value = value/10;
    d100  = value%10; value = value/10;
    d1000 = value%10; value = value/10;
    c1    = '0' + d1;
    c10   = '0' + d10;
    c100  = '0' + d100;
    c1000 = '0' + d1000;
}

/*
*************************************************************************
*  Segment::Cache Class
*************************************************************************
*/
void  Segment::Data::init(void) {
  _visible = true;
  _brightness = 0;
  memset(_buffer, 0, sizeof(_buffer));

}
void 	Segment::Data::reverse(void) {
  uint8_t tmp[DIGITS_PER_SEGMENT];
  memcpy(tmp,_buffer,sizeof(_buffer));
  for(int i=0; i<DIGITS_PER_SEGMENT; i++)  
    _buffer[i] = tmp[DIGITS_PER_SEGMENT-1 - i];
}

Segment::Data& Segment::Data::operator=(const Segment::Data& data) {
  _visible = data._visible;
  _brightness = data._brightness;
  memcpy(_buffer,data._buffer, sizeof(_buffer));
  return *this;
}

bool  Segment::Data::operator==(const Segment::Data& data) {
  if (_visible != data._visible) return false;
  if (_brightness != data._brightness) return false;
  for(int i=0;i<4;i++)
    if (_buffer[i] != data._buffer[i])
      return false;
  return true;
}

// Data  SEgment::Data::operator=(const Data&)

/*
*************************************************************************
*  Segment Class
*************************************************************************
*/

Device& Segment::device() { return sDevices[_iam];}
void    Segment::init(int iam) { 
    _iam = iam; 
    _data.init();
}

void    Segment::setBrightness(uint8_t brightness) {
    _data._brightness = brightness;
//    device().setBrightness(_data._brightness, _data._visible);
}

void    Segment::setVisible(bool visible) {
    _data._visible = visible;
//    device().setBrightness(_data._brightness, _data._visible);
}

void  Segment::drawDDDD(const TimeSpan& ts, int format) {
    Digits  days(ts.days());
    bool    showD = (format < 4);

    if (days.d1000) {
        encode(days.c1000, days.c100, days.c10, days.c1);
    } else {
        if (showD) {
            if (days.d100)     encode(days.c100, days.c10, days.c1, dchar);
            else if (days.d10) encode( days.c10,  days.c1,    space, dchar);
            else               encode(     space,  days.c1,    space, dchar); 
        } else {
            if (days.d100)     encode(space, days.c100, days.c10, days.c1);
            else if (days.d10) encode(space,     space, days.c10, days.c1);
            else               encode(space,     space,    space, days.c1); 
        }
    }
    setSegment();
};

void  Segment::drawHHMM(const TimeSpan& ts,int format) {
    Digits  hours(ts.hours());
    Digits  mins(ts.minutes());
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
    setSegment(colon);
};

void  Segment::drawSSUU(const TimeSpan& ts, uint8_t ms100, int format) {
    Digits  mins(ts.minutes());
    Digits  secs(ts.seconds());
    Digits  ms(ms100);
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
    setSegment(colon);
};

/*
******************************************************************************************
*/

void  Segment::drawDDDD(const DateTime& dt,int format) {
    Digits  years(dt.year());
    Digits  mons(dt.month());
    Digits  days(dt.day());
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
    setSegment(colon);
};

void  Segment::drawHHMM(const DateTime& dt, int format) {
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
    setSegment(colon);
};
 
void  Segment::drawSSUU(const DateTime& dt, uint8_t ms100, int format) {
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
    bool    colon         = showHoursMins || showMinsSecs;
    bool    blinking      = (format==1) || (format==4);
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
    setSegment(colon);
};

void  Segment::drawText(char* text, bool visible) {
    encode(text[3],text[2],text[1],text[0]);
    setVisible(visible);
/*
//    device().setBrightness(_data._brightness,visible);
    P("drawText: ");
    Serial.printf("%d |%c%c%c%c| ",_iam,text[3],text[2],text[2],text[0]);
    PVL(_data._brightness); 
*/
    setSegment(false,true);
}

void	Segment::setSegment(bool colon, bool print) {
	_data.reverse();
	if (colon) {
		uint8_t dots = 0x40;
  	for(int i = 0; i < DIGITS_PER_SEGMENT; ++i) {
	    _data._buffer[i] |= (dots & 0x80);
	    dots <<= 1;
	  }
	}

  if (_cache == _data) {
    if (print) {
      PL("setSegment: noChange");
    }
    return;
  }
  if (print) {
    PL("setSegment: Change");
  }

  _cache = _data;
  device().setBrightness(_data._brightness, _data._visible);
  device().setSegments(_data._buffer);
}

void  Segment::encode(char c3, char c2, char c1, char c0) {
  _data._buffer[3] = encodeChar(c3);
  _data._buffer[2] = encodeChar(c2);
  _data._buffer[1] = encodeChar(c1);
  _data._buffer[0] = encodeChar(c0);
}