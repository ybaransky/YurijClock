#include <Arduino.h>
#include <OneButton.h>
#include <TaskScheduler.h>
#include "Debug.h"
#include "Constants.h"
#include "RTCClock.h"

// task scheduler
Scheduler      *scheduler;

void initScheduler(void) {
  scheduler = new Scheduler; 
  scheduler->init();
}

// push button
OneButton      *button;
void oneButtonSingleClick() { PV(millis()); SPACE; PL("singleClick"); }
void oneButtonDoubleClick() { PV(millis()); SPACE; PL("doubleClick"); }
void oneButtonLongPress()   { PV(millis()); SPACE; PL("longPress"); }
void initOneButton() {
  button =  new OneButton(ONEBUTTON_PIN,false,false);  // D8
  button->attachClick(oneButtonSingleClick);
  button->attachDoubleClick(oneButtonDoubleClick);
  button->attachLongPressStart(oneButtonLongPress);
  button->setClickMs(400);
}

// RTC Clock
RTC *rtc;


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

void setup() {
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) continue;

  delay(2000);
  initOneButton();
  initScheduler();
  initRTC();

  P("compile time: "); PL(__TIMESTAMP__);
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
}