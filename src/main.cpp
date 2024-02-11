#include <Arduino.h>
#include <TaskScheduler.h>
#include "Action.h"
#include "Button.h"
#include "Debug.h"
#include "Constants.h"
#include "HTMLPages.h"
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
String          message;
Action          action;
WebServer       *server;

struct TickType {
  bool  sec;
  bool  ms100;
  bool  ms500;
  TickType(void) : sec(false),ms100(false),ms500(false) {}
};

WebServer* initWebServer(void) {
  WebServer* server = new WebServer(80); 
  
  P("access point: ssid="); P(config->_apSSID); P(" password=|"); P(config->_apPassword);PL("|");
  if (WiFi.softAP(config->_apSSID, config->_apPassword)) {
    P("Access point created with ip: "); PL(WiFi.softAPIP());
  } else {
    PL("Access point failed!");
  }
 
  server->on("/",        handleHome);

  server->on("/clock",   handleClock);
  server->on("/msgs",    handleMsgs);
  server->on("/view",    handleConfigView);
  server->on("/delete",  handleConfigDelete);

  server->on("/sync",     handleSync);
  server->on("/wifi",     handleWifi);

  server->on("/reboot",   handleReboot);
  server->begin();
  PL("Webserver started")

  return server;
}

/*
**********************************************************************
******************** Schduler ****************************************
**********************************************************************
*/

volatile bool EVENT_CLOCK_1_SEC  = false;
volatile bool EVENT_DEMO_START   = false;
volatile bool EVENT_INFO_START   = false;

void schedulerCB1sec(void) { EVENT_CLOCK_1_SEC = true;}
Task clock1sec(1000, TASK_FOREVER, &schedulerCB1sec);

Scheduler* initScheduler(void) {
  Scheduler* sch = new Scheduler();
  sch->init();
  sch->addTask(clock1sec);
  return sch;
}

void  reboot(const char* fcn) {
  P(fcn);SPACE;PL("reboot:");
  if (true) {
    delay(500);
    WiFi.forceSleepBegin();
    ESP.restart();
  }
}

/*
**********************************************************************
******************** setup   ****************************************
**********************************************************************
*/

void setup() {
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) 
    continue;

  for(int i=0;i<5;i++,delay(1000)) {
    Serial.print('.');
  }
  Serial.println();
  Serial.flush();
  PL("starting");
  
  config    = initConfig();
  button    = initOneButton();
  display   = initDisplay();
  scheduler = initScheduler();

  // this is the startup message
  message = config->getMsgStart();

  // get the rtc rtClock going, but is not found or working
  // then usef the soft rtClock, and set an error msg
  rtClock     = initRTClock();
  if (!rtClock->startTicking()) 
    clock1sec.enable();
  if (rtClock->lostPower()) {
    message = "Lost Pwr";
  }

  PL("");
  P("compile time: "); PL(__TIMESTAMP__);
  config->print();
//  Serial.flush();

  //display->test();

  // start the timer for the startup message
  // and then drop into the saved mode
  timer100ms.start(100);
  timer500ms.start(500);
  action.startInfo(message, 2); 

  server = initWebServer();
}

/*
**********************************************************************
************************ loop ****************************************
**********************************************************************
*/

void loop() {
  static const char* fcn="mainloop";
  bool updateDisplay = false;
  bool visible;
  TickType tickType;
  static TimeSpan ts;
  static DateTime rtc;
  static uint32_t freeHeap = ESP.getFreeHeap();

  #ifdef USE_ASYNC_WEBSERVER
  // do nothing
  #else
  server->handleClient();
  #endif
  button->tick();
  scheduler->execute();

  if (EVENT_CLOCK_1_SEC) {
    EVENT_CLOCK_1_SEC = false;
    tickType.sec  = true;
    updateDisplay = true;

    rtc = rtClock->now();  // only grab full date on second tick
    timer100ms.reset();   // reset 1/10 second timer on a full second tick
    timer500ms.reset();

    if (rtc.second()%20==0) {
      P(rtc.timestamp()); 
      P(" mode="); P(modeNames[config->getMode()]);
      P(" addr="); P(WiFi.softAPIP()); 
      P(" heap="); P(ESP.getFreeHeap());P("(");P(long(ESP.getFreeHeap()) - long(freeHeap));P(")");
      P(" clients=");P(WiFi.softAPgetStationNum());
      PL("");
    }

    // start this on a second boundary
    if (EVENT_DEMO_START) {
      EVENT_DEMO_START=false;
      action.startDemo();
    }

    // start this on a second boundary
    if (EVENT_INFO_START) {
      EVENT_INFO_START=false;
      action.startInfo(config->getMsgEnd(),10);
      action.print("***** starting action: ");
      display->refresh(fcn);
    }
  }
 
  // just the 1/10 second timer.
  if (timer100ms.tick()) {
    tickType.ms100 = true;
    if (config->isTenthSecFormat() || action.isDemoMode())
      updateDisplay  = true;
  }

  // this is the msg blinking rate
  if (timer500ms.tick()) {
    tickType.ms500 = true;
    updateDisplay  = true;
  }

  if (action.active()) {
    action.tick();
    if (action.expired()) {
      action.stop(fcn);
      action.restore();
      display->refresh(fcn);
    }
  }
 
  if (BUTTON_SINGLE_CLICK) {
    BUTTON_SINGLE_CLICK = false;
    PL("single button click ");
    action.startInfo(WiFi.softAPIP().toString(),10000);
    action.print("***** starting action: ");
    display->refresh(fcn);
    timer500ms.reset();
  }

  if (BUTTON_DOUBLE_CLICK) {
    BUTTON_DOUBLE_CLICK = false;
    PL("double button click");
    if (!action.isDemoMode())
      EVENT_DEMO_START = true;
  }

  if (BUTTON_LONG_CLICK) {
    BUTTON_LONG_CLICK = false;
    PL("Long button click");
    delay(2000);
    config->init();
    config->saveFile();
    reboot(fcn);
  }

 if (updateDisplay) {
    int count = timer100ms.count();
    DateTime expireTime;
    switch (config->getMode()) {
      case MODE_COUNTDOWN :
        expireTime = config->getTimeEndDT();
        ts = TimeSpan(expireTime.unixtime() - rtc.unixtime());
        display->showCount(ts, count ? 10-count : count);
        if (expireTime <= rtc)
          config->setMode(MODE_TEXT);
        break;
      case MODE_COUNTUP :
        ts = TimeSpan(rtc.unixtime() - config->getTimeStartDT().unixtime());
        display->showCount(ts, count);
        break;
      case MODE_CLOCK:
        display->showClock(rtc, count);
        break;
      case MODE_TEXT:
        // we could be doing a splash screen or an info msg (like show me the IP address)
        if (action.active()) {
          message = action.getMsg();
          visible = action.isBlinking() ? timer500ms.count()%2 : true;
        } else {
          message = config->getMsgEnd();
          visible = true;
        }
        display->showText(message, visible);
        break;
      case MODE_DEMO:
        // this has the countdown part and the text part
        if (action.getSecsRemaining()>=0) {
          if (true) {
            P(millis()); P(" sec="); P(rtc.second()); P(" sec rem=");P(action.getSecsRemaining()); P("  count=");PL(count);
          }
          ts = TimeSpan(action.getSecsRemaining());
          display->showCount(ts, count ? 10 - count : count);
        } else {
          message = action.getMsg();
          visible = action.isBlinking() ? timer500ms.count()%2 : true;
          display->showText(message, visible);
        }
        break;
      default :
        break;
    }
  }
}
