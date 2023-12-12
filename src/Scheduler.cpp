#include <TaskScheduler.h>
#include "Scheduler.h"

Scheduler* scheduler;

volatile bool EVENT_CLOCK_1_SEC = false;
volatile bool EVENT_ALARM_5_SEC = false;

void schedulerCB1sec(void) { EVENT_CLOCK_1_SEC = true;}
Task        clock1sec(1000, TASK_FOREVER, &schedulerCB1sec);

void schedulerCB5sec(void) { EVENT_ALARM_5_SEC = true;}
Task        alarm5sec(5000, TASK_ONCE, &schedulerCB5sec);

void initScheduler(void) {
  scheduler = new Scheduler();
  scheduler->init();
  scheduler->addTask(alarm5sec);
}

void start1SecScheduler(void) {
  scheduler->addTask(clock1sec);
  clock1sec.enable();
}

void tickScheduler(void) {
 if (scheduler) scheduler->execute();
}

void start5SecAlarm(void) {
  alarm5sec.enable();
}

