#include <Arduino.h>
#include "Constants.h"
#include "Segment.h"

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

void    Segment::init(int iam) { _iam = iam; _format = 0; }
void    Segment::setFormat(int format) { _format = format; }
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

void  Segment::drawDDDD(int days) {
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
    Digits digit(days);
    bool showLetter = (_format < 4);

    if (digit.d1000) {
        encode(digit.c1000, digit.c100, digit.c10, digit.c1);
    } else {
        if (showLetter) {
            if (digit.d100)     encode(digit.c100, digit.c10, digit.c1, dchar);
            else if (digit.d10) encode( digit.c10,  digit.c1,    space, dchar);
            else                encode(     space,  digit.c1,    space, dchar); 
        } else {
            if (digit.d100)     encode(space, digit.c100, digit.c10, digit.c1);
            else if (digit.d10) encode(space,      space, digit.c10, digit.c1);
            else                encode(space,      space,     space, digit.c1); 
        }
    }
    setSegment();
};
void  Segment::drawHHMM(int aHours, int aMinutes) {
    /*
    countdown display modes
    "0 dd D | hh:mm |  ss u",
    "1 dd D | hh:mm |    ss",
    "2 dd D | hh  H | mm:ss",
    "3 dd D | hh  H |    mm",
    "4   dd | hh:mm |  ss u",
    "5   dd | hh:mm |    ss",
    "6   dd |    hh | mm:ss",
    "7   dd |    hh |    mm"
    */
    Digits  hours(aHours);
    Digits  mins(aMinutes);
    bool    showHoursH    = (_format==2) || (_format==3);
    bool    showHours     = (_format==6) || (_format==7);
    bool    showHoursMins = (_format==0) || (_format==1) || (_format==4) || (_format==5);
    bool    colon = showHoursMins;

	if (showHoursMins) {
        if (hours.d100) encode(hours.c10, hours.c1, mins.c10, mins.c1);
        else            encode(    space, hours.c1, mins.c10, mins.c1);
    } else if (showHoursH) {
        if (hours.d100) encode(hours.c10, hours.c1, space, hchar);
        else            encode(    space, hours.c1, space, hchar);
    } else if (showHours) {
        if (hours.d100) encode(space, space, hours.c10, hours.c1);
        else            encode(space, space,     space, hours.c1);
    }
    setSegment(colon);
};

void  Segment::drawSSUU(int minutes, int seconds, uint8_t ms100) {
    /*
    countdown display modes
    "0 dd D | hh:mm |  ss u",
    "1 dd D | hh:mm |    ss",
    "2 dd D | hh  H | mm:ss",
    "3 dd D | hh  H |  mm N",
    "4   dd | hh:mm |  ss u",
    "5   dd | hh:mm |    ss",
    "6   dd |    hh | mm:ss",
    "7   dd |    hh |    mm"
    */
	// hours
    Digits mins(minutes);
    Digits secs(seconds);
    Digits ms(ms100);
    bool showMillis   = (_format==0) || (_format==4);
    bool showSecs     = (_format==1) || (_format==5);
    bool showMinsN    = (_format==3);
    bool showMins     = (_format==7);
    bool showMinsSecs = (_format==2) || (_format==6);
    bool colon = showMinsSecs;

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