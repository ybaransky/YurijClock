#include <Arduino.h>
#include "Segment.h"


/*
*************************************************************************
*  Segment Statics
*************************************************************************
*/

int     Segment::format = 1; 
#define COMMON_CLK   1
#ifdef COMMON_CLK
Device  Segment::devices[3] = {Device(D3,D4),Device(D3,D5),Device(D3,D6)};
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
}

/*
*************************************************************************
*  Segment Class
*************************************************************************
*/

void    Segment::init(int iam) { _iam = iam; }
Device& Segment::device() { return devices[_iam];}

void	Segment::setSegment(uint8_t* data, bool colon) {
	reverse(data);

	if (colon) {
		uint8_t dots = 0x40;
    	for(int i = 0; i < 4; ++i) {
	        data[i] |= (dots & 0x80);
	        dots <<= 1;
	    }
	}
	device().setSegments(data);
}

void  Segment::drawDDDD(int days) {
    /*
    display modes
    "dd D | hh:mm |  ss u",
    "dd D | hh:mm |    ss",
    "dd D | hh  H | mm:ss",
    "dd D | hh  H |  mm N",
    "  dd | hh:mm |  ss u",
    "  dd | hh:mm |    ss",
    "  dd |    hh | mm:ss",
    "  dd |    hh |    mm"
    */
    uint8_t data[4];
    Digits digits(days);

    if (digits.d1000) {
        data[3] = encodeDigit(digits.d1000);
        data[2] = encodeDigit(digits.d100);
        data[1] = encodeDigit(digits.d10);
        data[0] = encodeDigit(digits.d1);
    } else if (digits.d100) {
        data[3] = encodeDigit(digits.d100);
        data[2] = encodeDigit(digits.d10);
        data[1] = encodeDigit(digits.d1);
        data[0] = encodeChar('d');
    } else if (digits.d10) {
        data[3] = encodeDigit(digits.d10);
        data[2] = encodeDigit(digits.d1);
        data[1] = encodeChar(' ');
        data[0] = encodeChar('d');
    } else {
        data[3] = encodeChar(' ');
        data[2] = encodeDigit(digits.d1);
        data[1] = encodeChar(' ');
        data[0] = encodeChar('d');
    }
    setSegment(data);
};

void  Segment::drawHHMM(int hours, int minutes) {
    /*
    display modes
    "dd D | hh:mm |  ss u",
    "dd D | hh:mm |    ss",
    "dd D | hh  H | mm:ss",
    "dd D | hh  H |  mm N",
    "  dd | hh:mm |  ss u",
    "  dd | hh:mm |    ss",
    "  dd |    hh | mm:ss",
    "  dd |    hh |    mm"
    */
    uint8_t data[4];
    Digits digits(hours);

	// hours
    if (digits.d10) {
        data[3] = encodeDigit(digits.d10);
        data[2] = encodeDigit(digits.d1);
    } else if (digits.d1) {
        data[3] = encodeDigit(' ');
        data[2] = encodeDigit(digits.d1);
	} else {
        data[3] = encodeDigit(0);
        data[2] = encodeDigit(0);
	}

    digits.set(minutes);
	if (digits.d10) {
        data[1] = encodeDigit(digits.d10);
        data[0] = encodeDigit(digits.d1);
    } else {
        data[1] = encodeDigit(0);
        data[0] = encodeDigit(digits.d1);
    }
    setSegment(data,true);
};

void  Segment::drawSSUU(int seconds) {
    /*
    display modes
    "dd D | hh:mm |  ss u",
    "dd D | hh:mm |    ss",
    "dd D | hh  H | mm:ss",
    "dd D | hh  H |  mm N",
    "  dd | hh:mm |  ss u",
    "  dd | hh:mm |    ss",
    "  dd |    hh | mm:ss",
    "  dd |    hh |    mm"
    */
    uint8_t data[4];

	// hours
    Digits digits(seconds);
    if (digits.d10) {
        data[3] = encodeChar(' ');
        data[2] = encodeChar(' ');
        data[1] = encodeDigit(digits.d10);
        data[0] = encodeDigit(digits.d1);
    } else {
        data[3] = encodeChar(' ');
        data[2] = encodeChar(' ');
        data[1] = encodeChar(' ');
        data[0] = encodeDigit(digits.d1);
	}
    setSegment(data);
};

void  Segment::drawSSUU(int seconds,uint8_t ms100) {
    /*
    display modes
    "dd D | hh:mm |  ss u",
    "dd D | hh:mm |    ss",
    "dd D | hh  H | mm:ss",
    "dd D | hh  H |  mm N",
    "  dd | hh:mm |  ss u",
    "  dd | hh:mm |    ss",
    "  dd |    hh | mm:ss",
    "  dd |    hh |    mm"
    */
    uint8_t data[4];
	char seperator=' ';

	// hours
    Digits digits(seconds);
    if (digits.d10) {
        data[3] = encodeDigit(digits.d10);
        data[2] = encodeDigit(digits.d1);
        data[1] = encodeChar(seperator);
        data[0] = encodeDigit(ms100);
    } else {
        data[3] = encodeChar(' ');
        data[2] = encodeDigit(digits.d1);
        data[1] = encodeChar(seperator);
        data[0] = encodeDigit(ms100);
	}
    setSegment(data);
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

void	Segment::reverse(uint8_t* data) {
	uint8_t tmp[4];
	for(int i=0;i<4;i++) tmp[3-i] = data[i];
	for(int i=0;i<4;i++) data[i] = tmp[i];
}