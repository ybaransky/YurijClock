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
String          message,msgDemo,msgStart,msgFinal;

struct TickType {
  bool  sec;
  bool  ms100;
  bool  ms500;
  TickType(void) : sec(false),ms100(false),ms500(false) {}
};

struct DemoMode {
  void      start(const String& msg, int prevMode, ulong duration=5000);
  void      stop(void);
  bool      active(void) { return _active;}
  bool      expired(ulong ms=0);
  int       getPrevMode(void) { return _prevMode;}

  ulong     _start;
  ulong     _duration;
  int       _prevMode;
  String    _msg;
  bool      _active;
};

void  DemoMode::start(const String& msg, int prevMode, ulong duration) {
  _start = millis();
  _msg   = msg; 
  _prevMode = prevMode;
  _duration = duration;
  _active = true;
  P("starting Demo Mode:");PVL(_msg);

};
void  DemoMode::stop(void) {_active = false;}
bool  DemoMode::expired(ulong now) {
  if (_active) {
    if (!now) 
      now = millis();
    return now - _start > _duration;
  }
  return true;
}
DemoMode demoMode;

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
  message = config->_msgStart;

  // get the rtc rtClock going, but is not found or working
  // then usef the soft rtClock, and set an error msg
  rtClock     = initRTClock();
  if (!rtClock->startTicking()) 
    clock1sec.enable();
  if (rtClock->lostPower()) {
    message = "Lost Pwr";
  }

  Serial.flush();
  delay(1000);

  PL("");
  P("compile time: "); PL(__TIMESTAMP__);
  config->print();
  display->test();

  // start the timer for the startup message
  // and then drop into the saved mode
  timer100ms.start(100);
  timer500ms.start(500);
  demoMode.start(message,config->getMode()); 
  config->setMode(MODE_TEXT);
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
    timer100ms.reset();   // reset 1/10 second timer on a full second tick

    if (dt.second()%10==0) {
      config->print();
    }
    if (!dt.second()%5) {
      PL(dt.timestamp());
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
    updateDisplay  = true;

    // if there was a time limit, this must be start or demo style of text
    if (demoMode.active()) {
      if (demoMode.expired()) {
        demoMode.stop();
        config->setMode(demoMode.getPrevMode());
      }
    }
  }
 
  if (BUTTON_SINGLE_CLICK) {
    BUTTON_SINGLE_CLICK = false;
    PL("single button click ");
    config->setFormat(config->getNextFormat());
    config->print();
    display->clear(); // in case we caught a blink
  }

  if (BUTTON_DOUBLE_CLICK) {
    BUTTON_DOUBLE_CLICK = false;
    PL("double button click");
    config->setMode(config->getNextMode());
    config->print();
    display->clear(); // in case we caught a blink

    if (config->getMode()==MODE_TEXT) {
      message = msgFinal;
      timer500ms.reset();
    }
    PL("*************************************************************");
  }

  if (BUTTON_LONG_CLICK) {
    BUTTON_LONG_CLICK = false;
    PL("Long button click sarting demo mode");
    demoMode.start(msgDemo, config->getMode());
    timer500ms.reset();
    config->setMode(MODE_TEXT);
    config->print();
    /*
    DateTime dt(F(__DATE__),F(__TIME__));
    P("longPress"); P(" adjusting Datetime to: "); PL(dt.timestamp(DateTime::TIMESTAMP_FULL));
    rtClock->adjust(dt);
    */
  }

 if (updateDisplay) {
    int count = timer100ms.count();
    switch (config->getMode()) {
      case MODE_COUNTDOWN :
        ts = TimeSpan(DateTime(config->_timeFinal.c_str()).unixtime() - dt.unixtime());
        display->showCount(ts, count ? count : 10-count);
        break;
      case MODE_COUNTUP :
        ts = TimeSpan(dt.unixtime() - DateTime(config->_timeStart.c_str()).unixtime());
        display->showCount(ts, count);
        break;
      case MODE_CLOCK:
        display->showClock(dt, count);
        break;
      case MODE_TEXT:
        // force blinking (for now)
        display->showText(message, timer500ms.count()%2);
        break;
    }
  }
}