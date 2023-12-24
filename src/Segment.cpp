#include <Arduino.h>
#include <RTClib.h>
#include "Constants.h"
#include "Segment.h"
#include "Debug.h"

/* *************************************************************************
*  Segment::Data clas
*************************************************************************/
void  Segment::Data::init(void) {
  _visible    = true;
  _brightness = 7;
  memset(_buffer, 0, sizeof(_buffer));
}
void  Segment::Data::set(uint8_t* data, uint8_t brightness, bool visible) {
  memcpy(_buffer, data, sizeof(_buffer));
  _brightness = brightness;
  _visible    = visible;
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

void Segment::Data::print(const char* msg) {
  if (msg) Serial.printf("%s ",msg);
  Serial.printf("%02x %02x %02x %02x  %02x  %d ",
  _buffer[3],_buffer[2],_buffer[1],_buffer[0],_brightness,_visible);
}

/* *************************************************************************
*  statics
*************************************************************************/

#define COMMON_CLK   1
#ifdef COMMON_CLK
static Device  sDevices[N_SEGMENTS] = {Device(D3,D6),Device(D3,D5),Device(D3,D4)};
//static Device  sDevices[N_DEVICES] = {Device(D3,D4),Device(D3,D5),Device(D3,D6)};
#else
static Device sDevices[N_DEVICES] = {Device(D3,D4),Device(D5,D6),Device(RX,TX)};
#endif

#ifdef YURIJ
static char* reverse(char* data) {
  char tmp[DIGITS_PER_SEGMENT];
  memcpy(tmp,data,sizeof(tmp));
  for(int i=0; i<DIGITS_PER_SEGMENT; i++)  
    data[i] = tmp[DIGITS_PER_SEGMENT-1 - i];
  return data;
}
#endif

static void  addColon(uint8_t data[]) {
  uint8_t dots = 0x40;
  for(int i = 0; i < DIGITS_PER_SEGMENT; ++i) {
	  data[i] |= (dots & 0x80);
	  dots <<= 1;
	}
}
   
static uint8_t asciEncoding[96] = {
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
  return asciEncoding[16+i];
}

uint8_t Segment::encodeChar(char c) {
 if ((c < 32) || (c > (96+32))) c = 32;
 return asciEncoding[c-32];
}

void Segment::reverse(uint8_t *data, int n) {
  uint8_t tmp[CHARS_PER_MESSAGE];
  memcpy(tmp,data, n * sizeof(uint8_t));
  for(int i=0,m=n-1; i<n; i++)  
    data[i] = tmp[m - i];
}

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
}

void    Segment::setVisible(bool visible) {
    _data._visible = visible;
}

void  Segment::write(uint8_t data[], bool colon, uint8_t brightness, bool visible) {
  reverse(data);
  if (colon) addColon(data);

  _data.set(data,brightness,visible);

  if (false && _iam==0) {
    bool rc = _data == _cache;
    PV(_iam);SPACE;_data.print("data"); P(" "); _cache.print("cache"); P("  equal="); PL(rc);
  }

  if (_data == _cache) return;
  _cache = _data;

  Device& hardware = device();
  hardware.setBrightness(brightness,visible);
  hardware.setSegments(data);
};
