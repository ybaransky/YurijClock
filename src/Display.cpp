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

static const uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

void Display::init(void) {
  for(int i=0; i < N_SEGMENTS; i++) 
    _segments[i].setIndex(i);
}

void Display::test(void) {
  int values[] = {0000,1111,2222};
  bool on=true;
  for(int k=0; k < 8; k++) {
    for(int i=0;i<N_SEGMENTS;i++) {
      _segments[i].device().setBrightness(BRIGHTEST,on);
      _segments[i].device().showNumberDecEx(values[i],COLON,LEADING_ZERO);
    }
    delay(250);
    on = !on;
  }
  setBrightness(BRIGHTEST);
  return;
  for(int i=0;i<N_SEGMENTS;i++)
    _segments[i].test(200);
}

void  Display::setBrightness(uint8_t value) {
  for(int i=0;i<N_SEGMENTS;i++) 
    _segments[i].device().setBrightness(value);
}

void Display::showInteger(int32_t ival) {
    int parts[3];
    parts[2] =  ival / 100000000;
    parts[1] = (ival % 100000000) / 1000;
    parts[0] = (ival % 10000);
    for(int i=0; i<3; i++)
      _segments[i].device().showNumberDec(parts[i]);
}
/*

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

*/