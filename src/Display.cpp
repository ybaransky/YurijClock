#include <Arduino.h>
#include "Display.h"
#include "Constants.h"
#include "Debug.h"

/*
12/7/2023
For some reason, I can't create the TM1637Display objects via new. 
This needs to be understood 
*/

#define COMMON_CLK   1
#ifdef COMMON_CLK
  static Device sDevices[3] = {Device(D3,D4),Device(D3,D5),Device(D3,D6)};
#else
  static Device sDevice[3] = {Device(D3,D4),Device(D5,D6),Device(RX,TX)};
#endif
static const uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

Display* initDisplay(void) {
  Display* display = new Display();
  display->init();
  return display;
}

/*
*************************************************************************
*  Device Class
*************************************************************************
*/

void Display::init(void) {
  for(int i=0; i < N_SEGMENTS; i++) {
    Device& device = getDevice(i);
    device.clear();
  }
}

Device& Display::getDevice(int i) { 
  return sDevices[i]; 
}

void  Display::setBrightness(uint8_t brightness, bool on) {
  for(int i=0;i<N_SEGMENTS;i++) 
    getDevice(i).setBrightness(brightness, on);
}

void Display::test(void) {
  int values[] = {0000,1111,2222};
  bool zeroPad = true;
  bool on=true;
  for(int k=0; k < 8; k++) {
    for(int i=0;i<N_SEGMENTS;i++) {
      Device& device = getDevice(i);
      device.setBrightness(SEGMENT_BRIGHTEST,on);
      device.showNumberDecEx(values[i], SEGMENT_COLON, zeroPad);
    }
    delay(250);
    on = !on;
  }
  setBrightness(BRIGHTEST);
}

void Display::showInteger(int32_t ival) {
    int parts[3];
    parts[2] =  ival / 100000000;
    parts[1] = (ival % 100000000) / 10000;
    parts[0] = (ival % 10000);
    for(int i=0; i<3; i++)
      getDevice(i).showNumberDec(parts[i],false);
}

static const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};

void    testDevice(Device& device,int timeDelay) {
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
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