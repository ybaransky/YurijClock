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
Message         message;
Message         msgDemo;
Message         msgPerm;

struct TickType {
  bool  sec;
  bool  ms100;
  bool  ms500;
  TickType(void) : sec(false),ms100(false),ms500(false) {}
};

/*
**********************************************************************
******************** Schduler ****************************************
**********************************************************************
*/

volatile bool EVENT_CLOCK_1_SEC  = false;

void schedulerCB1sec(void) { EVENT_CLOCK_1_SEC = true;}
Task clock1sec(1000, TASK_FOREVER, &schedulerCB1sec);

Scheduler* initScheduler(void) {
  Scheduler* sch = new Scheduler();
  sch->init();
  sch->addTask(clock1sec);
  return sch;
}

/*
**********************************************************************
******************** setup   ****************************************
**********************************************************************
*/

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

  // get the rtc rtClock going, but is not found or working
  // then usef the soft rtClock, and set an error msg
  rtClock     = initRTClock();
  if (!rtClock->startTicking()) 
    clock1sec.enable();
  if (rtClock->lostPower()) {
    message.set("LostPwr",true);
  }

  // prepare the message object
  // then usef the soft rtClock, and set an error msg
  message.set(config->getMsgStart(),true);
  msgDemo.set("Fuc You",true);
  msgPerm.set("Good Luc",false);

  Serial.flush();
  delay(1000);

  PL("");
  P("compile time: "); PL(__TIMESTAMP__);
  config->print();
  display->test();
}

/*
**********************************************************************
******************** Schduler ****************************************
**********************************************************************
*/

void loop() {
  bool updateDisplay = false;
  TickType tickType;
  static TimeSpan ts;
  static DateTime dt;

  button->tick();
  scheduler->execute();

  if (EVENT_CLOCK_1_SEC) {
    EVENT_CLOCK_1_SEC = false;
    updateDisplay = true;
    tickType.sec  = true;

    dt = rtClock->now();  // only grab full date on second tick
    timer100ms.start(100);  // start 1/10 second timer on a full second tick
    if (dt.second()%10==0) {
      config->print();
    }
  }
 
  // just the 1/10 second timer.
  if (timer100ms.tick()) {
    if (config->isTenthSecFormat())
      updateDisplay  = true;
    tickType.ms100 = true;
  }

  // this is the msg blinking rate
  if (timer500ms.tick()) {
    if (message.isBlinking())
      updateDisplay  = true;
    tickType.ms500 = true;
    P(millis()); P(" 500ms "); PVL(timer500ms.count());

    if (timer500ms.finished()) {
      timer500ms.stop();
      config->restoreMode();
      display->clear();
      P(" Message over:  "); PL(message.text()); 
      PL("**********************");
      if (config->getMode() == MODE_MESSAGE) {
        message = msgPerm;
      }
    }
  }
 
  if (SINGLE_BUTTON_CLICK) {
    SINGLE_BUTTON_CLICK = false;
    PL("single button click ");
    config->incFormat();
//    display->refresh();
    config->print();
  }

  if (DOUBLE_BUTTON_CLICK) {
    DOUBLE_BUTTON_CLICK = false;
    PL("double button click");
    config->incMode();
    config->print();
    switch (config->getMode()) {
      case MODE_DEMO : 
        message = msgDemo;
        if (message.isBlinking())
          timer500ms.start(500,8000);
        break;
      case MODE_MESSAGE : 
        message = msgPerm;
        if (message.isBlinking())
          timer500ms.start(500);
        break;
      default :
        message.set("Good Luc",false);
        timer500ms.stop();
    }
    display->clear(); // in case we caught a blink
    PL("*************************************************************");
    PL("*************************************************************");
  }

  if (LONG_BUTTON_CLICK) {
    LONG_BUTTON_CLICK = false;
    PL("Long button click");
    config->setMode(MODE_DEMO);
    config->print();
    timer500ms.start(500,8000);
    message = msgDemo;
    message.print("main");
    /*
    DateTime dt(F(__DATE__),F(__TIME__));
    P("longPress"); P(" adjusting Datetime to: "); PL(dt.timestamp(DateTime::TIMESTAMP_FULL));
    rtClock->adjust(dt);
    */
  }

 if (updateDisplay) {
    int count;
    switch (config->getMode()) {
      case MODE_COUNTDOWN :
        count = timer100ms.count();
        ts = TimeSpan(DateTime(config->_isoFuture.c_str()).unixtime() - dt.unixtime());
//        Serial.printf("cuntdown: count=%s")
        display->showCountDown(ts, count ? count : 10-count);
        break;
      case MODE_CLOCK:
        count = timer100ms.count();
        display->showClock(dt, count);
        break;
      case MODE_MESSAGE:
      case MODE_DEMO:
        if (!tickType.ms100) { 
          timer500ms.print("main:");
          display->showText(message, timer500ms.finished() ? 1: timer500ms.count());
        }
        break;
    }
  }
}