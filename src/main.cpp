#include <Arduino.h>
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>
#include <TaskScheduler.h>
#include "Action.h"
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
  extern  void  handleRoot(void);
  extern  void  handleClockMode(void);

  extern  void  handleConfigSetup(void);
  extern  void  handleConfigSave(void);
  extern  void  handleConfigView(void);
  extern  void  handleConfigDelete(void);

  extern  void  handleSyncTime(void);
  extern  void  handleSyncTimeGet(void);

  extern  void  handleReboot(void);

  WebServer* server = new ESP8266WebServer(80); 
  
  P("access point: ssid="); P(config->_apSSID); P(" password=|"); P(config->_apPassword);PL("|");
  if (WiFi.softAP(config->_apSSID, config->_apPassword)) {
    P("Access point created with ip: "); PL(WiFi.softAPIP());
  } else {
    PL("Access point failed!");
  }
 
//  server->on("/",        handleConfigSetup);
//  server->on("/setup",   handleConfigSetup);
  server->on("/",        handleRoot);
  server->on("/mode",    handleClockMode);

  server->on("/setup",   handleConfigSetup);
  server->on("/save",    handleConfigSave);
  server->on("/view",    handleConfigView);
  server->on("/delete",  handleConfigDelete);

  server->on("/sync",    handleSyncTime);
  server->on("/syncget", handleSyncTimeGet);
  server->on("/reboot",  handleReboot);
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
  message = config->getMsgStart();

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
  //display->test();

  // start the timer for the startup message
  // and then drop into the saved mode
  timer100ms.start(100);
  timer500ms.start(500);
  action.info(message, 2000); 

  server = initWebServer();
}

/*
**********************************************************************
************************ loop ****************************************
**********************************************************************
*/

void loop() {
  const char* fcn="mainloop";
  bool updateDisplay = false;
  bool visible;
  TickType tickType;
  static TimeSpan ts;
  static DateTime rtc;

  server->handleClient();
  button->tick();
  scheduler->execute();

  if (EVENT_CLOCK_1_SEC) {
    EVENT_CLOCK_1_SEC = false;
    tickType.sec  = true;
    updateDisplay = true;

    rtc = rtClock->now();  // only grab full date on second tick
    timer100ms.reset();   // reset 1/10 second timer on a full second tick

    if (rtc.second()%5==0) {
      P(rtc.timestamp()); P(" mode="); P(modeNames[config->getMode()]); SPACE
      P("addr="); P(WiFi.softAPIP()); P(" clients=");P(WiFi.softAPgetStationNum());
      PL("");
    }

    // start this on a second boundary
    if (EVENT_DEMO_START) {
      EVENT_DEMO_START=false;
      action.demo(config->getMsgEnd());
      timer500ms.reset();
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
      action.stop();
      action.setPrevDisplay();
      display->refresh(fcn);
    }
  }
 
  if (BUTTON_SINGLE_CLICK) {
    BUTTON_SINGLE_CLICK = false;
    PL("single button click ");
    action.info(WiFi.softAPIP().toString(),10000);
    action.print("***** starting action: ");
    display->refresh(fcn);
    timer500ms.reset();
  }

  if (BUTTON_DOUBLE_CLICK) {
    BUTTON_DOUBLE_CLICK = false;
    PL("double button click");
    action.demo(config->getMsgEnd());
    display->refresh(fcn);
    timer500ms.reset();
  }

  if (BUTTON_LONG_CLICK) {
    BUTTON_LONG_CLICK = false;
    PL("Long button click");
    delay(2000);
    config->init();
    config->saveFile();
    extern void reboot(void);
    reboot();
  }

 if (updateDisplay) {
    int count = timer100ms.count();
    DateTime expireTime;
    switch (config->getMode()) {
      case MODE_COUNTDOWN :
        #ifdef YURIJ
        if (action.active()) {
          expireTime = action.getExpireTime();
        } else {
          expireTime = DateTime(config->getTimeEnd().c_str());
        }
        #endif
        expireTime = DateTime(config->getTimeEnd().c_str());
        ts = TimeSpan(expireTime.unixtime() - rtc.unixtime());
        display->showCount(ts, count ? 10-count : count);
        if (expireTime <= rtc)
          config->setMode(MODE_TEXT);
        break;
      case MODE_COUNTUP :
        ts = TimeSpan(rtc.unixtime() - DateTime(config->getTimeStart().c_str()).unixtime());
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
          P(millis()); SPACE; PL(action.getSecsRemaining());
          ts = TimeSpan(action.getSecsRemaining());
          display->showCount(ts, count ? 10-count : count);
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
void  reboot(void) {
//  delay(2000);
  PL("about to reboot")
  if (false) {
    WiFi.forceSleepBegin();
    ESP.restart();
  }
}
