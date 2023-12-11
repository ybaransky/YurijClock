#include <Arduino.h>
#include "Button.h"
#include "Debug.h"
#include "Constants.h"
#include "RTClock.h"
#include "Display.h"
#include "Config.h"

/*
**********************************************************************
******************** Globals *****************************************
**********************************************************************
*/

RTClock     *rtClock;   
OneButton   *button;
Display     *display;
Config      *config;
RTTimer     rtTimer;


void timeToWedding(void) {
  DateTime fut(config->_future);
  DateTime cur(rtClock->now());
  TimeSpan span(fut.unixtime() - cur.unixtime());
  P("from now till then "); 
  P("days=");  P(span.days()); SPACE;
  P("hours="); P(span.hours()); SPACE;
  P("mins=");  P(span.minutes()); SPACE;
  P("secs=");  PL(span.seconds());
}

void setup() {
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) continue;
  Serial.flush();
  delay(1000);

  PL("starting");
  
  config  = initConfig();
  button  = initOneButton();
  display = initDisplay();
  rtClock = initRTClock();

  Serial.flush();
  delay(1000);

  PL("");
  P("compile time: "); PL(__TIMESTAMP__);

  display->test();
}

void loop() {
  static int count = 0;
  static TimeSpan span;
  static int format = 0;

  button->tick();
  rtClock->tick();

  if (CLOCK_TICK_1_SEC) {
    CLOCK_TICK_1_SEC = false;
    rtTimer.start(millis(),100);

    DateTime fut(config->_future);
    DateTime cur = rtClock->now();
    span = TimeSpan(fut.unixtime() - cur.unixtime());

    display->showTimeSpan(span);
    if (1==count%10)
      timeToWedding();
    count++;
  }
  if (rtTimer.tick(millis())) {
    display->showTimeSpan(span,10 - rtTimer.count());
  }

  if (SINGLE_BUTTON_CLICK) {
    SINGLE_BUTTON_CLICK = false;
    P("single button click ");
    PV("  current "); PV(format); 
    format++;
    format = format%7;
    PV("  new "); PVL(format);
    display->setFormat(format);
  }

  if (DOUBLE_BUTTON_CLICK) {
    DOUBLE_BUTTON_CLICK = false;
    PL("double button click");
  }

  if (LONG_BUTTON_CLICK) {
    LONG_BUTTON_CLICK = false;
    DateTime dt(F(__DATE__),F(__TIME__));
    P("longPress"); P(" adjusting Datetime to: "); PL(dt.timestamp(DateTime::TIMESTAMP_FULL));
    rtClock->adjust(dt);
  }
}