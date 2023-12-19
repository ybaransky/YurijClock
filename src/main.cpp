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
Message         msgStart;
Message         msgFinal;

struct TickType {
  bool  sec;
  bool  ms100;
  bool  ms500;
  TickType(void) : sec(false),ms100(false),ms500(false) {}
};

struct ModeState {
  int       _mode;
  String    _msg;
  bool      _blinking;
};

ModeState   prevMode;

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

  // this is the startup message
  message.set(config->_msgStart,true);

  // get the rtc rtClock going, but is not found or working
  // then usef the soft rtClock, and set an error msg
  rtClock     = initRTClock();
  if (!rtClock->startTicking()) 
    clock1sec.enable();
  if (rtClock->lostPower()) {
    message.set("LostPwr",true);
  }

  Serial.flush();
  delay(1000);

  PL("");
  P("compile time: "); PL(__TIMESTAMP__);
  config->print();
  display->test();

  // start the timer for the startup message
  // and then drop into the saved mode
  timer500ms.start(500,5000);
  prevMode._mode = config->_mode;
}

/*
**********************************************************************
************************ loop ****************************************
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
    tickType.sec  = true;
    updateDisplay = true;

    dt = rtClock->now();  // only grab full date on second tick
    timer100ms.start(100);  // start 1/10 second timer on a full second tick

    if (dt.second()%10==0) {
      config->print();
    }
  }
 
  // just the 1/10 second timer.
  if (timer100ms.tick()) {
    tickType.ms100 = true;
    if (config->isTenthSecFormat())
      updateDisplay  = true;
  }

  // this is the msg blinking rate
  if (timer500ms.tick()) {
    tickType.ms500 = true;

    if (message.isBlinking())
      updateDisplay  = true;

    // if there was a time limit, this must be start or demo style of text
    if (timer500ms.finished()) {
      timer500ms.stop();
      config->setMode(prevMode._mode);
      display->clear();
      P(" Message over:  "); PL(message.text()); 
      PL("**********************");
      if (config->getMode() == MODE_MESSAGE) {
        message = msgPerm;
      }
    }
  }
 
  if (BUTTON_SINGLE_CLICK) {
    BUTTON_SINGLE_CLICK = false;
    PL("single button click ");
    config->setFormat(config->getNextFormat());
//    display->refresh();
    config->print();
  }

  if (BUTTON_DOUBLE_CLICK) {
    BUTTON_DOUBLE_CLICK = false;
    PL("double button click");
    config->setMode(config->getNextMode());
    config->print();
    if (config->getMode()) {
      message = msgFinal;
      if (message.isBlinking())
        timer500ms.start(500);
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