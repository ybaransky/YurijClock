#include <Arduino.h>
#include <OneButton.h>
#include <TaskScheduler.h>
#include "Debug.h"
#include "Constants.h"
#include "RTCClock.h"
#include "Display.h"

RTC         *rtc;   
OneButton   *button;
Scheduler   *scheduler;
Display     *display;

// push button
void oneButtonSingleClick() { PV(millis()); SPACE; PL("singleClick"); }
void oneButtonDoubleClick() { PV(millis()); SPACE; PL("doubleClick"); }
void oneButtonLongPress()   { 
  DateTime dt(F(__DATE__),F(__TIME__));
  PV(millis()); SPACE; P("longPress"); 
  P(" adjusting Datetime to: "); PL(dt.timestamp(DateTime::TIMESTAMP_FULL));
  rtc->adjust(dt);
}

void initOneButton() {
  button =  new OneButton(ONEBUTTON_PIN,false,false);  // D8
  button->attachClick(oneButtonSingleClick);
  button->attachDoubleClick(oneButtonDoubleClick);
  button->attachLongPressStart(oneButtonLongPress);
  button->setClickMs(400);
}

volatile bool SECOND_TICK;
void IRAM_ATTR callback1Hz(void) {
    SECOND_TICK = true;
}

void callback1HzScheduler(void) {
    SECOND_TICK = true;
}

Task task(2000, TASK_FOREVER, &callback1HzScheduler);
void initRTC() {
  rtc = new RTC();
  rtc->init(); 
  if (!rtc->attachSQWInterrupt(callback1Hz)) {
    scheduler->addTask(task);
    task.enable();
  }
}

void  initDisplay(void) {
  display = new Display();
  display->init();
}

bool MILLIS_100_TICK = false;
void callback100ms(void) { MILLIS_100_TICK = true; }
Task task2(100, TASK_FOREVER, &callback100ms);
void initScheduler(void) {
  scheduler = new Scheduler; 
  scheduler->init();
  scheduler->addTask(task2);
  task2.enable();
}

void setup() {
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) continue;

  Serial.flush();
  delay(1000);
  PL("starting");
  initOneButton();
  initScheduler();
  initRTC();
  initDisplay();

  Serial.flush();
  delay(1000);
  PL("");
  P("compile time: "); PL(__TIMESTAMP__);

  display->test();
}

void loop() {
  // check for button events
  button->tick();
  scheduler->execute();

  if (SECOND_TICK) {
    DateTime dt = rtc->now();
    String str = dt.timestamp(DateTime::TIMESTAMP_FULL);
    PV(millis()); SPACE; PL(str);
    SECOND_TICK = false;
  }

  if (MILLIS_100_TICK) {
    static int count=0;
    display->showInteger(count++);
    MILLIS_100_TICK = false;
  }
}