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
  msTimer.execute();

  if (EVENT_CLOCK_1_SEC) {
    EVENT_CLOCK_1_SEC = false;
    updateDisplay = true;
    secondTick = true;
    dt = rtClock->now();  // only grab full date on second tick
    msTimer.start(100);
    if (dt.second()%10==0) {
      PVL(dt.second());
      config->print();
    }
  }
 
  if (EVENT_CLOCK_100_MS) {
    EVENT_CLOCK_100_MS = false;
    updateDisplay = true;
  }
  
  if (EVENT_ALARM_5_SEC) {
    EVENT_ALARM_5_SEC = false;
    P(" Message over:  "); PL(message.text()); 
    alarm5sec.disable();
    config->restoreMode();
    display->reset();
    updateDisplay = true;
  }

  if (updateDisplay) {
    int count = msTimer.count();
    switch (config->getMode()) {
      case MODE_COUNTDOWN :
        ts = TimeSpan(DateTime(config->_future).unixtime() - dt.unixtime());
        if (secondTick) {
          Serial.printf("d=%d h=%d m=%d s=%d\n",
            ts.days(),ts.hours(),ts.minutes(),ts.seconds());
        }
        display->showTime(ts, count ? count : 10-count);
        break;
      case MODE_COUNTUP:
        display->showTime(dt, count);
        break;
      case MODE_MESSAGE:
        if (secondTick) display->showText(dt, message);
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
    PL("double button click setting msg for 5 secs");
    message.set("1234567890ab",true);
    message.print();
    config->setMode(MODE_MESSAGE);
    alarm5sec.enableDelayed(5000);
    config->print();
  }

  if (LONG_BUTTON_CLICK) {
    LONG_BUTTON_CLICK = false;
    DateTime dt(F(__DATE__),F(__TIME__));
    P("longPress"); P(" adjusting Datetime to: "); PL(dt.timestamp(DateTime::TIMESTAMP_FULL));
    rtClock->adjust(dt);
  }
}