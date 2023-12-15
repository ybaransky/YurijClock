#include <Arduino.h>
#include <TaskScheduler.h>
#include "Button.h"
#include "Debug.h"
#include "Constants.h"
#include "RTClock.h"
#include "Timer.h"
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
Timer           timer100ms;
Timer           timer500ms;
DisplayMsg      message;

volatile bool EVENT_CLOCK_1_SEC  = false;

void schedulerCB1sec(void) { EVENT_CLOCK_1_SEC = true;}
Task clock1sec(1000, TASK_FOREVER, &schedulerCB1sec);

Scheduler* initScheduler(void) {
  Scheduler* sch = new Scheduler();
  sch->init();
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

  message.set(config->getText(),true);

  Serial.flush();
  delay(1000);

  PL("");
  P("compile time: "); PL(__TIMESTAMP__);
  config->print();
  display->test();
}

void loop() {
  bool updateDisplay = false;
  bool secondTick = false;
  static TimeSpan ts;
  static DateTime dt;

  button->tick();
  scheduler->execute();

  if (EVENT_CLOCK_1_SEC) {
    EVENT_CLOCK_1_SEC = false;

    updateDisplay = true;
    secondTick = true;
    dt = rtClock->now();  // only grab full date on second tick
    timer100ms.start(100);
    if (dt.second()%10==0) {
      PVL(dt.second());
      config->print();
    }
  }
 
  // just the 1/10 second timer.
  if (timer100ms.tick()) {
    updateDisplay = true;
  }

  // this is how msg blinking rate
  if (timer500ms.tick()) {
    updateDisplay = true;
    if (timer500ms.finished()) {
      timer500ms.stop();
      config->restoreMode();
      display->reset();
      P(" Message over:  "); PL(message.text()); 
    }
  }

  if (updateDisplay) {
    int count = timer100ms.count();
    switch (config->getMode()) {
      case MODE_COUNTDOWN :
        ts = TimeSpan(DateTime(config->_future.c_str()).unixtime() - dt.unixtime());
        display->showCountDown(ts, count ? count : 10-count);
        break;
      case MODE_CLOCK:
        display->showClock(dt, count);
        break;
      case MODE_MESSAGE:
        display->showMessage(message,
          timer500ms.finished() ? 1: timer500ms.count());
        break;
    }
  }

  if (SINGLE_BUTTON_CLICK) {
    SINGLE_BUTTON_CLICK = false;
    PL("single button click ");
    config->incFormat();
    display->refresh();
    config->print();
  }

  if (DOUBLE_BUTTON_CLICK) {
    DOUBLE_BUTTON_CLICK = false;
    PL("double button click");
    config->incMode();
    display->refresh();
    config->print();
  }

  if (LONG_BUTTON_CLICK) {
    LONG_BUTTON_CLICK = false;
    message.set("Fuc You",true);
    message.print();
    config->setMode(MODE_MESSAGE);
    timer500ms.start(500,8000);
    config->print();
    /*
    DateTime dt(F(__DATE__),F(__TIME__));
    P("longPress"); P(" adjusting Datetime to: "); PL(dt.timestamp(DateTime::TIMESTAMP_FULL));
    rtClock->adjust(dt);
    */
  }
}