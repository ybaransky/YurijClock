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
WebServer       *server;

Timer           timer100ms;
Timer           timer500ms;
String          actionMessage;
Action          action;

/*
struct TickType {
  bool  sec;
  bool  ms100;
  bool  ms500;
  TickType(void) : sec(false),ms100(false),ms500(false) {}
};
*/

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
  server->on("/dir",     handleDirectory);
  server->on("/file",    handleFile);
  server->on("/view",    handleViewConfig);
  server->on("/delete",  handleDeleteConfig);

  server->on("/sync",    handleSync);
  server->on("/wifi",    handleWifi);

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

volatile bool EVENT_CLOCK_1_SEC = false;
volatile bool EVENT_DEMO_START  = false;
volatile bool EVENT_TEXT_START  = false;
volatile bool EVENT_ADDR_START  = false;

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

  for(int i=0;i<1;i++,delay(1000)) {
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
  actionMessage = config->getMsgStart();

  // get the rtc rtClock going, but is not found or working
  // then usef the soft rtClock, and set an error msg
  rtClock = initRTClock();
  if (rtClock->start()) {
    if (rtClock->lostPower()) 
      actionMessage = "Lobatt";
  } else {
    clock1sec.enable();
  }
  // actionMessage = "no  rtcloc";

  PL("");
  P("compile time: "); PL(__TIMESTAMP__);
  config->print();
//  Serial.flush();

  //display->test();

  // start the timer for the startup message
  // and then drop into the saved mode
  timer100ms.start(100);
  timer500ms.start(500);
  action.start(Action::TEXT, actionMessage, 5); 

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
      action.start(Action::DEMO,config->getMsgEnd(),5,true);
    }

    // start this on a second boundary
    if (EVENT_TEXT_START) {
      EVENT_TEXT_START=false;
      action.start(Action::TEXT, actionMessage, 5);
    }

    if (EVENT_ADDR_START) {
      EVENT_ADDR_START=false;
      action.start(Action::ADDR, actionMessage, 8);
    }
  }
 
  // just the 1/10 second timer.
  if (timer100ms.tick()) {
    updateDisplay = (config->isTenthSecFormat() || action.isDemoMode()) ? true : false;
  }

  // this is the msg blinking rate
  if (timer500ms.tick()) {
    updateDisplay  = true;
  }

  if (action.active()) {
    if (action.isOver()) {
      action.stop(fcn);
      display->refresh(fcn);
      EVENT_ADDR_START = false;
      EVENT_TEXT_START = false;
      EVENT_DEMO_START = false;
    }
  }
 
  if (BUTTON_SINGLE_CLICK) {
    BUTTON_SINGLE_CLICK = false;
    PL("single button click ");
    EVENT_ADDR_START = action.active() ? false : true;
  }

  if (BUTTON_DOUBLE_CLICK) {
    BUTTON_DOUBLE_CLICK = false;
    PL("double button click");
    EVENT_TEXT_START = action.active() ? false : true;
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
    ulong timeSinceStart;
    char buffer[13];
    IPAddress ipaddr;
    DateTime expireTime;
    if (action.active()) {
      // we are in some demo-ish mode
      switch (action.type()) {
        case Action::DEMO :
          if (action.showDemoMessage()) {
            display->showText(config->getMsgEnd(), action.isBlinking() ? !timer500ms.count()%2 : true);
          } else {
            ts = TimeSpan(action.getSecsRemaining());
            display->showCount(ts, count ? 10 - count : count, 0);
          }
          break;
        case Action::TEXT :
          display->showText(action.getMessage(), action.isBlinking() ? (!timer500ms.count()%2) : true);
          break;

        case Action::ADDR :
          ipaddr = WiFi.softAPIP();
          timeSinceStart = (millis() - action.startTime()) / 1000;
          if ( (timeSinceStart >= 0) && (timeSinceStart < 2)) 
            sprintf(buffer,"  IPAddr%4d", ipaddr[0]);
          else if ( (timeSinceStart >= 2) && (timeSinceStart < 4)) 
            sprintf(buffer,"  IPAddr%4d", ipaddr[1]);
          else if ( (timeSinceStart >= 4) && (timeSinceStart < 6))
            sprintf(buffer,"  IPAddr%4d", ipaddr[2]);
          else if (timeSinceStart >= 6) 
            sprintf(buffer,"  IPAddr%4d", ipaddr[3]);
          display->showText(String(buffer));
          break;

        default :
          P("bad action type:"); PL(action.type());
      }
    } else {
      int format = config->getFormat();
      switch (config->getMode()) {
        case MODE_COUNTDOWN :
          expireTime = config->getTimeEndDT();
          if (expireTime <= rtc) {
            display->showText(config->getMsgEnd(), !timer500ms.count()%2 );
          } else {
            ts = TimeSpan(expireTime.unixtime() - rtc.unixtime());
            display->showCount(ts, count ? 10-count : count, format);
          }
          break;
        case MODE_COUNTUP :
          ts = TimeSpan(rtc.unixtime() - config->getTimeStartDT().unixtime());
          display->showCount(ts, count, format);
          break;
        case MODE_CLOCK:
          display->showClock(rtc, count, format);
          break;
        default :
          PL("illegal mode");
          break;
      }
    }
  }
}