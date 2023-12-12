#include <Arduino.h>
#include "Scheduler.h"
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
RTTimer     ms100Timer;


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
  
  initScheduler();

  config    = initConfig();
  button    = initOneButton();
  display   = initDisplay();
  rtClock   = initRTClock();

  Serial.flush();
  delay(1000);

  PL("");
  P("compile time: "); PL(__TIMESTAMP__);

  display->test();
}

void loop() {
  bool updateDisplay = false;
  static TimeSpan span;
  static DateTime current;

  button->tick();
  rtClock->tick();

  if (EVENT_CLOCK_1_SEC) {
    EVENT_CLOCK_1_SEC = false;
    updateDisplay = true;
    current = rtClock->now();  // only grab full date on second tick
    ms100Timer.start(millis(),100);
  }
  
  if (EVENT_ALARM_5_SEC) {
    ;
  }

  if (ms100Timer.tick(millis())) {
    updateDisplay = true;
  }

  if (updateDisplay) {
    int count = ms100Timer.count();  // number of 100ms that passed
    switch (display->getMode()) {
      case MODE_COUNTDOWN :
        span = TimeSpan(DateTime(config->_future).unixtime() - current.unixtime());
        display->showTime(span, count ? count : 10-count);
        break;
      case MODE_COUNTUP:
        display->showTime(current, count);
        break;
      case MODE_MESSAGE:
        display->showMessage(config->_message, current.second() % 2);
    }
  }

  if (SINGLE_BUTTON_CLICK) {
    SINGLE_BUTTON_CLICK = false;
    PL("single button click ");
    display->incFormat();
    display->refresh();
  }

  if (DOUBLE_BUTTON_CLICK) {
    DOUBLE_BUTTON_CLICK = false;
    PL("double button click ");
    display->incMode();
    display->refresh();
  }

  if (LONG_BUTTON_CLICK) {
    LONG_BUTTON_CLICK = false;
    DateTime dt(F(__DATE__),F(__TIME__));
    P("longPress"); P(" adjusting Datetime to: "); PL(dt.timestamp(DateTime::TIMESTAMP_FULL));
    rtClock->adjust(dt);
  }
}