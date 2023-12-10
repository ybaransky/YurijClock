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
RTTimer     timer;


void timeToWedding(void) {
  DateTime fut(config->_future);
  DateTime cur = rtClock->now();
  PVL(fut.timestamp(DateTime::TIMESTAMP_FULL)) ;
  PVL(cur.timestamp(DateTime::TIMESTAMP_FULL));
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

  timeToWedding();
  display->test();
}

void loop() {
  static int count = 0;
  static TimeSpan span;

  button->tick();
  rtClock->tick();

  if (CLOCK_TICK_1_SEC) {
    CLOCK_TICK_1_SEC = false;
    timer.start(millis(),100);

    DateTime fut(config->_future);
    DateTime cur = rtClock->now();
    span = TimeSpan(fut.unixtime() - cur.unixtime());

    display->showTimeSpan(span,0);
    if (1==count%10)
      timeToWedding();
    count++;
  }

  if (timer.tick(millis())) {
    display->showTimeSpan(span,10 - timer.count());
  }

}