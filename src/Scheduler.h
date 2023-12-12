#pragma once

extern void initScheduler(void);
extern void tickScheduler(void);
extern void start1SecScheduler(void);
extern void start5SecAlarm(void);

/*
extern Task       clock1sec(unsigned long, long, TaskCallback);
extern Task       alarm5sec(unsigned long, long, TaskCallback);
*/

extern  volatile  bool EVENT_CLOCK_1_SEC;
extern  volatile  bool EVENT_ALARM_5_SEC;