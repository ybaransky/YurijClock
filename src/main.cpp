#include <Arduino.h>
#include <TaskScheduler.h>
#include "Button.h"
#include "Debug.h"
#include "Constants.h"
#include "RTClock.h"
#include "Display.h"
#include "Config.h"

class ClockError {
  public:
      void  set(const char*);
      void  clear(void);
      bool  error(void);
      const char* msg(void);
  private:
      char  _msg[13];
      bool  _flag;
};
/*
**********************************************************************
******************** Globals *****************************************
**********************************************************************
*/

Scheduler   *scheduler;
RTClock     *rtClock;   
OneButton   *button;
Display     *display;
Config      *config;
RTTimer     ms100Timer;
ClockError  clkError;

volatile bool EVENT_CLOCK_1_SEC = false;
volatile bool EVENT_ALARM_5_SEC = false;

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

void ClockError::set(const char *msg) {
  strncpy(_msg,msg,sizeof(char)*13);
  _flag = true;
  alarm5sec.enable();
}

const char* ClockError::msg(void)   { return _msg;}
bool        ClockError::error(void) { return _flag;}
void        ClockError::clear(void) { 
  _flag = false;
  alarm5sec.disable();
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
    clkError.set("LostPwr");
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

  if (EVENT_CLOCK_1_SEC) {
    EVENT_CLOCK_1_SEC = false;
    updateDisplay = true;
    current = rtClock->now();  // only grab full date on second tick
    ms100Timer.start(millis(),100);
    if (current.second()%10==0) {
      PVL(current.second());
    }
  }
  
  if (EVENT_ALARM_5_SEC) {
    EVENT_ALARM_5_SEC = false;
    P(" Got an error:  "); PL(clkError.msg()); 
    clkError.clear();
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
    if (display->getFormat() == 3 || display->getFormat() == 5)
      clkError.set("test error");
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