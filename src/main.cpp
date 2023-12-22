#include <Arduino.h>
#include <TaskScheduler.h>
#include "Action.h"
#include "Button.h"
#include "Debug.h"
#include "Constants.h"
#include "RTClock.h"
#include "Timer.h"
#include "Display.h"
#include "Config.h"
#include "WebServer.h"

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

void  setUserHandlers() {
  extern  void  handleConfigClock(void);
  extern  void  handleConfigSave(void);
  extern  void  handleConfigView(void);
  extern  void  handleConfigDelete(void);
  extern  void  handleConfigReboot(void);
  PL("adding handlers")
//  server->on("/",       handleConfigClock);
  PL("done handlers")
/*
  server->on("/clock",  handleConfigClock);
  server->on("/save",   handleConfigSave);
  server->on("/view",   handleConfigView);
  server->on("/delete", handleConfigDelete);
  server->on("/reboot", handleConfigReboot);
*/
}

WebServer* initWebServer(void) {
  WebServer* server = new ESP8266WebServer(80); 
  
  P("access point: ssid="); P(config->_apSSID); P(" password="); PL(config->_apPassword);
  if (WiFi.softAP(config->_apSSID, config->_apPassword)) {
    P("Access point created with ip: "); PL(WiFi.softAPIP());
  } else {
    PL("Access point failed!");
  }
  setUserHandlers();
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

#define PWEB
#ifdef PWEB
ESP8266WebServer *wserver;
#else
ESP8266WebServer wserver(80);
#endif
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

//  server    = initWebServer();

  PL("");
  P("compile time: "); PL(__TIMESTAMP__);
  config->print();
  //display->test();

  // start the timer for the startup message
  // and then drop into the saved mode
  timer100ms.start(100);
  timer500ms.start(500);
  action.splash(message,config->getMode(),1000); 
  config->setMode(MODE_TEXT);

  const char* ssid = "YuriCloc";
  const char* password = "12345678";

extern void handleRoot(void);
extern void handleConfigClock(void);
extern void handleConfigView(void);

    #ifdef PWEB
    wserver = new ESP8266WebServer(80);
    WiFi.softAP(ssid, password);
 //   wserver->on("/", handleRoot);
    wserver->on("/",     handleConfigClock);
    wserver->on("/view", handleConfigView);
    wserver->begin();
    #else
    WiFi.softAP(ssid, password);
    wserver.on("/", handleRoot);
    wserver.begin();
    #endif
    PL("wserver started");
}

/*
**********************************************************************
************************ loop ****************************************
**********************************************************************
*/

void loop() {
  bool updateDisplay = false;
  bool visible;
  TickType tickType;
  static TimeSpan ts;
  static DateTime dt;

 // server->handleClient();
 #ifdef PWEB
 wserver->handleClient();
 #else
 wserver.handleClient();
 #endif

  button->tick();
  scheduler->execute();

  if (EVENT_CLOCK_1_SEC) {
    EVENT_CLOCK_1_SEC = false;
    tickType.sec  = true;
    updateDisplay = true;

    dt = rtClock->now();  // only grab full date on second tick
    timer100ms.reset();   // reset 1/10 second timer on a full second tick

    /*
    if (dt.second()%10==0) {
      config->print();
    }
    */
    if (dt.second()%2==0) {
      P(dt.timestamp()); P(WiFi.softAPIP()); PL(modeNames[config->getMode()]);
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
  }

  if (action.active()) {
    if (action.expired()) {
      action.stop();
      config->setMode(action.getPrevMode());
    }
  }
 
  if (BUTTON_SINGLE_CLICK) {
    BUTTON_SINGLE_CLICK = false;
    PL("single button click ");
    config->setFormat(config->getNextFormat());
    config->print();
//    display->clear(); // in case we caught a blink
  }

  if (BUTTON_DOUBLE_CLICK) {
    BUTTON_DOUBLE_CLICK = false;
    PL("double button click");
    config->setMode(config->getNextMode());
    config->print();
//    display->clear(); // in case we caught a blink

    if (config->getMode()==MODE_TEXT) {
      message = config->_msgFinal;
      timer500ms.reset();
      P("setting to test mode... ");PVL(message);
    }
    PL("*************************************************************");
  }

  if (BUTTON_LONG_CLICK) {
    BUTTON_LONG_CLICK = false;
    PL("Long button click sarting demo mode");
    action.demo(config->_msgFinal, config->getMode(), dt);
    action.print("***** starting action: ");
    config->setMode(MODE_COUNTDOWN);
    config->print();
    timer500ms.reset();
    /*
    DateTime dt(F(__DATE__),F(__TIME__));
    P("longPress"); P(" adjusting Datetime to: "); PL(dt.timestamp(DateTime::TIMESTAMP_FULL));
    rtClock->adjust(dt);
    */
  }

 if (updateDisplay) {
    int count = timer100ms.count();
    DateTime expireTime;
    switch (config->getMode()) {
      case MODE_COUNTDOWN :
        if (action.active()) {
          expireTime = action.getExpireTime();
        } else {
          expireTime = DateTime(config->_timeFinal.c_str());
        }
        ts = TimeSpan(expireTime.unixtime() - dt.unixtime());
        display->showCount(ts, count ? 10-count : count);
        if (expireTime <= dt)
          config->setMode(MODE_TEXT);
        break;
      case MODE_COUNTUP :
        ts = TimeSpan(dt.unixtime() - DateTime(config->_timeStart.c_str()).unixtime());
        display->showCount(ts, count);
        break;
      case MODE_CLOCK:
        display->showClock(dt, count);
        break;
      case MODE_TEXT:
        if (action.active()) {
            message = action.getMsg();
            visible = action.isBlinking() ? timer500ms.count()%2 : true;
            action.print();
        } else {
          message = config->_msgFinal;
          visible = true;
        }
        display->showText(message, visible);
        break;
    }
  }
}

void  reboot(void) {
//  delay(2000);
  WiFi.forceSleepBegin();
  ESP.restart();
  //digitalWrite(D0, LOW);
}
#ifdef YURIJ
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "YuriCloc";
const char* password = "12345678";

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "Hello World!");
}

void setup() {
  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}
#endif

