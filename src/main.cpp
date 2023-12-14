#include <Arduino.h>
#include <TaskScheduler.h>
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

Scheduler       *scheduler;
RTClock         *rtClock;   
OneButton       *button;
Display         *display;
Config          *config;
RTTimer         msTimer;
DisplayMsg      message;

volatile bool EVENT_CLOCK_1_SEC  = false;
volatile bool EVENT_CLOCK_100_MS = false;
volatile bool EVENT_ALARM_5_SEC  = false;

void schedulerCB1sec(void) { EVENT_CLOCK_1_SEC = true;}
Task clock1sec(1000, TASK_FOREVER, &schedulerCB1sec);

void schedulerCB5sec(void) { EVENT_ALARM_5_SEC = true;}
Task alarm5sec(5000, TASK_FOREVER, &schedulerCB5sec);

Scheduler* initScheduler(void) {
  Scheduler* sch = new Scheduler();
  sch->init();
  sch->addTask(alarm5sec);
  sch->addTask(clock1sec);
  return sch;
}
void setup() {
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) continue;
  Serial.flush();
  delay(1000);

  PL("starting");
  
  config    = initConfig();
  button    = initOneButton();
  display   = initDisplay();
  scheduler = initScheduler();

  // get the rtc clock going, but is not found or working
  // then usef the soft clock, and set an error msg
  rtClock   = initRTClock();
  if (!rtClock->startTicking()) 
    clock1sec.enable();
  if (rtClock->lostPower()) {
    message.set("LostPwr",true);
  }

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
  scheduler->execute();
  msTimer.execute();

  if (EVENT_CLOCK_1_SEC) {
    EVENT_CLOCK_1_SEC = false;
    updateDisplay = true;
    current = rtClock->now();  // only grab full date on second tick
    msTimer.start(100);
    if (current.second()%10==0) {
      PVL(current.second());
    }
  }
 
  if (EVENT_CLOCK_100_MS) {
    EVENT_CLOCK_100_MS = false;
    updateDisplay = true;
  }
  
  if (EVENT_ALARM_5_SEC) {
    EVENT_ALARM_5_SEC = false;
    P(" Got an error:  "); PL(message.text()); 
    alarm5sec.disable();
    display->restoreMode();
    updateDisplay = true;
  }

  if (updateDisplay) {
    int count = msTimer.count();
    switch (display->getMode()) {
      case MODE_COUNTDOWN :
        span = TimeSpan(DateTime(config->_future).unixtime() - current.unixtime());
        display->showTime(span, count ? count : 10-count);
        break;
      case MODE_COUNTUP:
        display->showTime(current, count);
        break;
      case MODE_MESSAGE:
        display->showMessage(current, message);
        break;
    }
  }

  if (SINGLE_BUTTON_CLICK) {
    SINGLE_BUTTON_CLICK = false;
    PL("single button click ");
    display->incFormat();
    display->refresh();
    if (display->getFormat() == 3 || display->getFormat() == 5) {
      message.set("test error",true);
    }

  }

  if (DOUBLE_BUTTON_CLICK) {
    DOUBLE_BUTTON_CLICK = false;
    PL("double button click setting msg for 5 secs");
    message.set("test error",true);
    display->setMode(MODE_MESSAGE);
    alarm5sec.enable();
    /*
    display->incMode();
    display->refresh();
    */
  }

  if (LONG_BUTTON_CLICK) {
    LONG_BUTTON_CLICK = false;
    DateTime dt(F(__DATE__),F(__TIME__));
    P("longPress"); P(" adjusting Datetime to: "); PL(dt.timestamp(DateTime::TIMESTAMP_FULL));
    rtClock->adjust(dt);
  }
}