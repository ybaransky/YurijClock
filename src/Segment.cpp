#include <Arduino.h>
#include <RTClib.h>
#include "Constants.h"
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
    "4     MM |    DD | hh:mm",  // blinkind hh:mm colon
    "5  MM:DD | hh:mm | ss  u",
    "7  MM:DD | hh:mm |    ss",
    "8  MM:DD |    hh | mm:ss",
    "9  MM:DD |    hh |    mm",
    "10    DD | hh:mm | ss  u",
    "11    DD | hh:mm |    ss",
    "12    DD |    hh | mm:ss",
    "13    DD |    hh |    mm",
*/
 
/*
*************************************************************************
*  Segment Statics
*************************************************************************
*/

char    Segment::space = ' '; 
char    Segment::dchar = 'd';
char    Segment::hchar = 'h';
char    Segment::nchar = 'n';

#define COMMON_CLK   1
#ifdef COMMON_CLK
Device  Segment::devices[3] = {Device(D3,D6),Device(D3,D5),Device(D3,D4)};
//Device  Segment::devices[3] = {Device(D3,D4),Device(D3,D5),Device(D3,D6)};
#else
Device Segment::devices[3] = {Device(D3,D4),Device(D5,D6),Device(RX,TX)};
#endif

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
*  Segment Class
*************************************************************************
*/

void    Segment::init(int iam, int* formats) { 
    _iam = iam; 
    for(int i=0; i<N_DISPLAY_MODES; i++) _formats[i] = formats[i]; 
}

void    Segment::setFormat(int displayMode, int format ) {
     _formats[displayMode] = format;
}

Device& Segment::device() { return devices[_iam];}

void	Segment::setSegment(bool colon) {
	reverse();
	if (colon) {
		uint8_t dots = 0x40;
    	for(int i = 0; i < 4; ++i) {
	        _data[i] |= (dots & 0x80);
	        dots <<= 1;
	    }
	}
    if (changed()) {
        saveToCache();
    	device().setSegments(_data);
    }
}

bool    Segment::changed(void) {
    for(int i=0;i<4;i++) 
        if (_data[i] != _cache[i]) return true;
    return false;
}

void    Segment::saveToCache(void) {
    for(int i=0; i<4; i++) _cache[i] = _data[i];
}

void  Segment::drawDDDD(TimeSpan ts) {
    Digits  days(ts.days());
    int     format = _formats[DISPLAY_COUNTDOWN];
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

void  Segment::drawHHMM(TimeSpan ts) {
    Digits  hours(ts.hours());
    Digits  mins(ts.minutes());
    int     format = _formats[DISPLAY_COUNTDOWN];
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

void  Segment::drawSSUU(TimeSpan ts, uint8_t ms100) {
    Digits  mins(ts.minutes());
    Digits  secs(ts.seconds());
    Digits  ms(ms100);
    int     format = _formats[DISPLAY_COUNTDOWN];
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

void  Segment::drawDDDD(DateTime dt) {
    Digits  years(dt.year());
    Digits  mons(dt.month());
    Digits  days(dt.day());
    int     format = _formats[DISPLAY_COUNTUP];
    bool    showYears    = (format==0) || (format==1) || (format==2);
    bool    showMons     = (format==3) || (format==4);
    bool    showDays     = (format==9) || (format==10) || (format==11) || (format==12);
    bool    showMonsDays = (format==5) || (format==6) || (format==7) || (format==8);
    bool    colon = showMonsDays;

    if (showYears) {
        encode(years.c1000, years.c100, years.c10, years.c1);
    } else if (showMons) {
        if (mons.c10) encode(space, space, mons.c10, mons.c1);
        else          encode(space, space,    space, mons.c1);
    } else if (showDays) {
        if (days.c10) encode(space, space, days.c10, days.c1);
        else          encode(space, space,   space, days.c1);
    } else {
        if (mons.c10) encode(mons.c10, mons.c1, days.c10, days.c1);
        else          encode(   space, mons.c1, days.c10, days.c1);
    }
    setSegment(colon);
};

/* 
    countup display modes
    "0   YYYY | MM:DD | hh:mm",
    "1   YYYY | MM:DD | hh:mm",  // blinking hh:mm colon
    "2   YYYY |    MM |    DD",
    "3     MM |    DD | hh:mm",
    "4     MM |    DD | hh:mm",  // blinkind hh:mm colon
    "5  MM:DD | hh:mm | ss  u",
    "6  MM:DD | hh:mm |    ss",
    "7  MM:DD |    hh | mm:ss",
    "8  MM:DD |    hh |    mm",
    "9     DD | hh:mm | ss  u",
    "10    DD | hh:mm |    ss",
    "11    DD |    hh | mm:ss",
    "12    DD |    hh |    mm",
*/
 
void  Segment::drawHHMM(DateTime dt) {
    Digits  mons(dt.month());
    Digits  days(dt.day());
    Digits  hours(dt.hour());
    Digits  mins(dt.minute());
    int     format = _formats[DISPLAY_COUNTUP];
    bool    showMonsDays  =  (format==0) || (format==1);
    bool    showMons      =  (format==2);
    bool    showDays      =  (format==3) || (format==4);
    bool    showHoursMins =  (format==5) || (format==6) || (format==9) || (format==10);
    bool    showHours     =  (format==7) || (format==8) || (format==11) || (format==12);
    bool    colon = showMonsDays || showHoursMins;

    if (showMonsDays) {
        if (mons.d10)  encode(mons.c10, mons.c1, days.c10, days.c1);
        else           encode(   space, mons.c1, days.c10, days.c1);
    } else if (showMons) {
        if (mons.c10) encode(space, space, mons.c10, mons.c1);
        else          encode(space, space,    space, mons.c1);
    } else if (showDays) {
        if (days.c10) encode(space, space, days.c10, days.c1);
        else          encode(space, space,    space, days.c1);
    } else if (showHoursMins) {
        if (hours.d10)  encode(hours.c10, hours.c1, mins.c10, mins.c1);
        else            encode(    space, hours.c1, mins.c10, mins.c1);
    } else if (showHours) {
        if (hours.c10) encode(space, space, hours.c10, hours.c1);
        else           encode(space, space,     space, hours.c1);
    }
    setSegment(colon);
};
 
void  Segment::drawSSUU(DateTime dt, uint8_t ms100) {
    Digits  days(dt.day());
    Digits  hours(dt.hour());
    Digits  mins(dt.minute());
    Digits  secs(dt.second());
    Digits  ms(ms100);

    int     format = _formats[DISPLAY_COUNTUP];
    bool    showHoursMins = (format==0) || (format==1) || (format==3) || (format==4);
    bool    showDays      = (format==2);
    bool    showMillis    = (format==5) || (format==9);
    bool    showSecs      = (format==6) || (format==10);
    bool    showMinsSecs  = (format==7) || (format==11);
    bool    showMins      = (format==12);
    bool    blinking      = (format==1) || (format==4);
    bool    colon = showHoursMins || showMinsSecs;

    if (showHoursMins) {
        if (hours.d10) encode(hours.c10, hours.c1, mins.c10, mins.c1);
        else           encode(    space, hours.c1, mins.c10, mins.c1);
    } else if (showDays) {
        if (days.c10) encode(space, space, days.c10, days.c1);
        else          encode(space, space,    space, days.c1);
    } else if (showMillis) {
        if (secs.c10) encode(secs.c10, secs.c1, space, ms.c1);
        else          encode(   space, secs.c1, space, ms.c1);
    } else if (showSecs) {
        if (secs.c10) encode(space, space, secs.c10, secs.c1);
        else          encode(space, space,    space, secs.c1);
    } else if (showMinsSecs) {
        if (mins.d10)  encode(mins.c10, mins.c1, secs.c10, secs.c1);
        else           encode(   space, mins.c1, secs.c10, secs.c1);
    } else if (showMins) {
        if (mins.c10) encode(space, space, mins.c10, mins.c1);
        else          encode(space, space,    space, mins.c1);
    }
    setSegment(colon);
};

/*
*************************************************************************
*  statics
*************************************************************************
*/

uint8_t Segment::asciEncoding[96] = {
  /*       a
   *      ---
   *  f |  g | b
   *      ---
   *  e |    | c
   *      ---
   *       d
   * a== bit 0... g==bit7
   */

  	0b00000000, /* (space) this is 32 */
   	0b10000110, /* ! */
   	0b00100010, /* " */
	0b01111110, /* # */
    0b01101101, /* $ */
    0b11010010, /* % */
	0b01000110, /* & */
	0b00100000, /* ' */
	0b00101001, /* ( */
	0b00001011, /* ) */
	0b00100001, /* * */
	0b01110000, /* + */
	0b00010000, /* , */
	0b01000000, /* - */
	0b10000000, /* . */
	0b01010010, /* / */
	0b00111111, /* 0 */
	0b00000110, /* 1 */
	0b01011011, /* 2 */
	0b01001111, /* 3 */
	0b01100110, /* 4 */
	0b01101101, /* 5 */
	0b01111101, /* 6 */
	0b00000111, /* 7 */
	0b01111111, /* 8 */
	0b01101111, /* 9 */
	0b00001001, /* : */
	0b00001101, /* ; */
	0b01100001, /* < */
	0b01001000, /* = */
	0b01000011, /* > */
	0b11010011, /* ? */
	0b01011111, /* @ */
	0b01110111, /* A */
	0b01111100, /* B */
	0b00111001, /* C */
	0b01011110, /* D */
	0b01111001, /* E */
	0b01110001, /* F */
	0b00111101, /* G */
	0b01110110, /* H */
	0b00110000, /* I */
	0b00011110, /* J */
	0b01110101, /* K */
	0b00111000, /* L */
	0b00010101, /* M */
	0b00110111, /* N */
	0b00111111, /* O */
	0b01110011, /* P */
	0b01101011, /* Q */
	0b00110011, /* R */
	0b01101101, /* S */
	0b01111000, /* T */
	0b00111110, /* U */
	0b00111110, /* V */
	0b00101010, /* W */
	0b01110110, /* X */
	0b01101110, /* Y */
	0b01011011, /* Z */
	0b00111001, /* [ */
	0b01100100, /* \ */
	0b00001111, /* ] */
	0b00100011, /* ^ */
	0b00001000, /* _ */
	0b00000010, /* ` */
	0b01011111, /* a */
	0b01111100, /* b */
	0b01011000, /* c */
	0b01011110, /* d */
	0b01111011, /* e */
	0b01110001, /* f */
	0b01101111, /* g */
	0b01110100, /* h */
	0b00010000, /* i */
	0b00001100, /* j */
	0b01110101, /* k */
	0b00110000, /* l */
	0b00010100, /* m */
	0b01010100, /* n */
	0b01011100, /* o */
	0b01110011, /* p */
	0b01100111, /* q */
	0b01010000, /* r */
	0b01101101, /* s */
	0b01111000, /* t */
	0b00011100, /* u */
	0b00011100, /* v */
	0b00010100, /* w */
	0b01110110, /* x */
	0b01101110, /* y */
	0b01011011, /* z */
	0b01000110, /* { */
	0b00110000, /* | */
	0b01110000, /* } */
	0b00000001, /* ~ */
	0b00000000, /* (del) */
};
 
uint8_t Segment::encodeDigit(uint8_t i) {
	if (i > 9) i = 0;
   uint8_t e = asciEncoding[16+i];
//   Serial.printf("%d --> 0b",i); Serial.println(e,BIN);
   return e;
}

uint8_t Segment::encodeChar(char c) {
   if ((c < 32) || (c > (96+32))) c = 32;
   return asciEncoding[c-32];
}

void	Segment::reverse(void) {
	uint8_t tmp[4];
	for(int i=0;i<4;i++) tmp[3-i] = _data[i];
	for(int i=0;i<4;i++) _data[i] = tmp[i];
}

void  Segment::encode(char c3, char c2, char c1, char c0) {
    _data[3] = encodeChar(c3);
    _data[2] = encodeChar(c2);
    _data[1] = encodeChar(c1);
    _data[0] = encodeChar(c0);
}