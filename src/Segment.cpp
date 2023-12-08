#include <Arduino.h>
#include "Segment.h"
#include "Debug.h"

#define COMMON_CLK   1
#ifdef COMMON_CLK
  static Device myDevices[3] = {Device(D3,D4),Device(D3,D5),Device(D3,D6)};
#else
  static Device sDevice[3] = {Device(D3,D4),Device(D5,D6),Device(RX,TX)};
#endif

static const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};


void    Segment::setIndex(int i) { _index = i; }
Device& Segment::device(void) {return myDevices[_index]; }

void    Segment::test(int timeDelay) {
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  Device& device = this->device();
  int k;
  device.setBrightness(0x0f);

  // All segments on
  PL("all segments on");
  device.setSegments(data);
  delay(timeDelay);

  // Selectively set different digits
  PL("selectively set differnet digits");
  for(k=0;k<4;k++)
    data[k] = device.encodeDigit(k);
  device.setSegments(data);
  delay(timeDelay);

  device.clear();
  device.setSegments(data+2, 2, 2);
  delay(timeDelay);

  device.clear();
  device.setSegments(data+2, 2, 1);
  delay(timeDelay);

  device.clear();
  device.setSegments(data+1, 3, 1);
  delay(timeDelay);

  // Show decimal numbers with/without leading zeros
  PL("Show decimal numbers with/without leading zero");
  device.showNumberDec(0, false); // Expect: ___0
  delay(timeDelay);
  device.showNumberDec(0, true);  // Expect: 0000
  delay(timeDelay);
  device.showNumberDec(1, false); // Expect: ___1
	delay(timeDelay);
  device.showNumberDec(1, true);  // Expect: 0001
  delay(timeDelay);
  device.showNumberDec(301, false); // Expect: _301
  delay(timeDelay);
  device.showNumberDec(301, true); // Expect: 0301
  delay(timeDelay);
  device.clear();
  device.showNumberDec(14, false, 2, 1); // Expect: _14_
  delay(timeDelay);
  device.clear();
  device.showNumberDec(4, true, 2, 2);  // Expect: __04
  delay(timeDelay);
  device.showNumberDec(-1, false);  // Expect: __-1
  delay(timeDelay);
  device.showNumberDec(-12);        // Expect: _-12
  delay(timeDelay);
  device.showNumberDec(-999);       // Expect: -999
  delay(timeDelay);
  device.clear();
  device.showNumberDec(-5, false, 3, 0); // Expect: _-5_
  delay(timeDelay);
  device.showNumberHexEx(0xf1af);        // Expect: f1Af
  delay(timeDelay);
  device.showNumberHexEx(0x2c);          // Expect: __2C
  delay(timeDelay);
  device.showNumberHexEx(0xd1, 0, true); // Expect: 00d1
  delay(timeDelay);
  device.clear();
  device.showNumberHexEx(0xd1, 0, true, 2); // Expect: d1__
  delay(timeDelay);
  
	// Run through all the dots
  PL("Run through all the dots");
	for(k=0; k <= 4; k++) {
		device.showNumberDecEx(0, (0x80 >> k), true);
		delay(timeDelay);
	}

  // Brightness Test
  PL("Brightness Test");
  for(k = 0; k < 4; k++)
	  data[k] = 0xff;
  for(k = 0; k < 7; k++) {
    device.setBrightness(k);
    device.setSegments(data);
    delay(timeDelay);
  }
  
  // On/Off test
  PL("On/Off test");
  for(k = 0; k < 4; k++) {
    device.setBrightness(7, false);  // Turn off
    device.setSegments(data);
    delay(timeDelay);
    device.setBrightness(7, true); // Turn on
    device.setSegments(data);
    delay(timeDelay);  
  }
  // Done!
  PL("done");
  device.setSegments(SEG_DONE);
  PVL(millis());
}