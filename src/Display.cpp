#include <Arduino.h>
#include "Debug.h"
#include "Display.h"
#include "Constants.h"
#include "Debug.h"

/*
12/7/2023
For some reason, I can't create the TM1637Display objects via new. 
This needs to be understood 
*/

static const bool LEADING_ZERO = true;
static const uint8_t BRIGHTEST = 7;
static const uint8_t DIMMEST = 1;
static const uint8_t COLON = 0x40; // 0b01000000;
static const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};
static const uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

static void testSegment(TM1637Display*,int);
static void testSegment0(TM1637Display&,int);

// #define COMMON_CLK   1
#ifdef COMMON_CLK
  static Segment sSegments[3] = {Segment(D3,D4),Segment(D3,D5),Segment(D3,D6)};
#else
  static Segment segments[3] = {Segment(D3,D4),Segment(D5,D6),Segment(RX,TX)};
#endif

void Display::init(void) {
  int values[] =  {0000,1111,2222};
  bool on=true;
  for(int k=0; k < 16; k++) {
    for(int i=0;i<N_SEGMENTS;i++) {
      Segment& segment=device(i);
      segment.setBrightness(BRIGHTEST,on);
      segment.showNumberDecEx(values[i],COLON,LEADING_ZERO);
    }
    delay(1000);
    on = !on;
  }
}

void  Display::setBrightness(uint8_t v) {
  for(int i=0;i<N_SEGMENTS;i++) device(i).setBrightness(v);
}

Segment& Display::device(int i) {
  return sSegments[i];
}

void Display::test(void) {
    testSegment(&device(2),250);
    testSegment(&device(1),250);
    testSegment(&device(0),250);
}


void displayNumber(Segment &display, int number) {
    display.showNumberDecEx(number,COLON);
}

void displayCount(TM1637Display &display, int count, int increment) {
    display.showNumberDecEx(count,COLON);
}

void testSegment(TM1637Display *display,int timeDelay) {
    uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  int k;
  display->setBrightness(0x0f);

  // All segments on
  PL("all segments on");
  display->setSegments(data);
  delay(timeDelay);

  // Selectively set different digits
  PL("selectively set differnet digits");
  for(int i=0;i<4;i++)
    data[i] = display->encodeDigit(i);
  display->setSegments(data);
  delay(timeDelay);

  display->clear();
  display->setSegments(data+2, 2, 2);
  delay(timeDelay);

  display->clear();
  display->setSegments(data+2, 2, 1);
  delay(timeDelay);

  display->clear();
  display->setSegments(data+1, 3, 1);
  delay(timeDelay);

  // Show decimal numbers with/without leading zeros
  PL("Show decimal numbers with/without leading zero");
  display->showNumberDec(0, false); // Expect: ___0
  delay(timeDelay);
  display->showNumberDec(0, true);  // Expect: 0000
  delay(timeDelay);
    display->showNumberDec(1, false); // Expect: ___1
	delay(timeDelay);
  display->showNumberDec(1, true);  // Expect: 0001
  delay(timeDelay);
  display->showNumberDec(301, false); // Expect: _301
  delay(timeDelay);
  display->showNumberDec(301, true); // Expect: 0301
  delay(timeDelay);
  display->clear();
  display->showNumberDec(14, false, 2, 1); // Expect: _14_
  delay(timeDelay);
  display->clear();
  display->showNumberDec(4, true, 2, 2);  // Expect: __04
  delay(timeDelay);
  display->showNumberDec(-1, false);  // Expect: __-1
  delay(timeDelay);
  display->showNumberDec(-12);        // Expect: _-12
  delay(timeDelay);
  display->showNumberDec(-999);       // Expect: -999
  delay(timeDelay);
  display->clear();
  display->showNumberDec(-5, false, 3, 0); // Expect: _-5_
  delay(timeDelay);
  display->showNumberHexEx(0xf1af);        // Expect: f1Af
  delay(timeDelay);
  display->showNumberHexEx(0x2c);          // Expect: __2C
  delay(timeDelay);
  display->showNumberHexEx(0xd1, 0, true); // Expect: 00d1
  delay(timeDelay);
  display->clear();
  display->showNumberHexEx(0xd1, 0, true, 2); // Expect: d1__
  delay(timeDelay);
  
	// Run through all the dots
  PL("Run through all the dots");
	for(k=0; k <= 4; k++) {
		display->showNumberDecEx(0, (0x80 >> k), true);
		delay(timeDelay);
	}

  // Brightness Test
  PL("Brightness Test");
  for(k = 0; k < 4; k++)
	data[k] = 0xff;
  for(k = 0; k < 7; k++) {
    display->setBrightness(k);
    display->setSegments(data);
    delay(timeDelay);
  }
  
  // On/Off test
  PL("On/Off test");
  for(k = 0; k < 4; k++) {
    display->setBrightness(7, false);  // Turn off
    display->setSegments(data);
    delay(timeDelay);
    display->setBrightness(7, true); // Turn on
    display->setSegments(data);
    delay(timeDelay);  
  }
  // Done!
  PL("done");
  display->setSegments(SEG_DONE);
  PVL(millis());
}

void testSegment0(TM1637Display &display,int timeDelay) {
    uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  int k;
  display.setBrightness(0x0f);

  // All segments on
  PL("all segments on");
  display.setSegments(data);
  delay(timeDelay);

  // Selectively set different digits
  PL("selectively set differnet digits");
  data[0] = display.encodeDigit(0);
  data[1] = display.encodeDigit(1);
  data[2] = display.encodeDigit(2);
  data[3] = display.encodeDigit(3);
  display.setSegments(data);
  delay(timeDelay);

  display.clear();
  display.setSegments(data+2, 2, 2);
  delay(timeDelay);

  display.clear();
  display.setSegments(data+2, 2, 1);
  delay(timeDelay);

  display.clear();
  display.setSegments(data+1, 3, 1);
  delay(timeDelay);

  // Show decimal numbers with/without leading zeros
  PL("Show decimal numbers with/without leading zero");
  display.showNumberDec(0, false); // Expect: ___0
  delay(timeDelay);
  display.showNumberDec(0, true);  // Expect: 0000
  delay(timeDelay);
	display.showNumberDec(1, false); // Expect: ___1
	delay(timeDelay);
  display.showNumberDec(1, true);  // Expect: 0001
  delay(timeDelay);
  display.showNumberDec(301, false); // Expect: _301
  delay(timeDelay);
  display.showNumberDec(301, true); // Expect: 0301
  delay(timeDelay);
  display.clear();
  display.showNumberDec(14, false, 2, 1); // Expect: _14_
  delay(timeDelay);
  display.clear();
  display.showNumberDec(4, true, 2, 2);  // Expect: __04
  delay(timeDelay);
  display.showNumberDec(-1, false);  // Expect: __-1
  delay(timeDelay);
  display.showNumberDec(-12);        // Expect: _-12
  delay(timeDelay);
  display.showNumberDec(-999);       // Expect: -999
  delay(timeDelay);
  display.clear();
  display.showNumberDec(-5, false, 3, 0); // Expect: _-5_
  delay(timeDelay);
  display.showNumberHexEx(0xf1af);        // Expect: f1Af
  delay(timeDelay);
  display.showNumberHexEx(0x2c);          // Expect: __2C
  delay(timeDelay);
  display.showNumberHexEx(0xd1, 0, true); // Expect: 00d1
  delay(timeDelay);
  display.clear();
  display.showNumberHexEx(0xd1, 0, true, 2); // Expect: d1__
  delay(timeDelay);
  
	// Run through all the dots
  PL("Run through all the dots");
	for(k=0; k <= 4; k++) {
		display.showNumberDecEx(0, (0x80 >> k), true);
		delay(timeDelay);
	}

  // Brightness Test
  PL("Brightness Test");
  for(k = 0; k < 4; k++)
	data[k] = 0xff;
  for(k = 0; k < 7; k++) {
    display.setBrightness(k);
    display.setSegments(data);
    delay(timeDelay);
  }
  
  // On/Off test
  PL("On/Off test");
  for(k = 0; k < 4; k++) {
    display.setBrightness(7, false);  // Turn off
    display.setSegments(data);
    delay(timeDelay);
    display.setBrightness(7, true); // Turn on
    display.setSegments(data);
    delay(timeDelay);  
  }
  // Done!
  PL("done");
  display.setSegments(SEG_DONE);
  PVL(millis());
}

