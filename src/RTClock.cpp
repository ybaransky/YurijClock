#include <TaskScheduler.h>
#include "RTClock.h"
#include "Debug.h"
#include "Constants.h"

volatile bool   CLOCK_TICK_1_SEC = false;

/*
**********************************************************************
******************* Scheudler ****************************************
**********************************************************************
*/

Scheduler*  scheduler=nullptr;      // only used if the rtc is broken
void        schedulerCB1sec(void) { CLOCK_TICK_1_SEC = true;}
Task        schedulerTask1sec(1000, TASK_FOREVER, &schedulerCB1sec);

/*
**********************************************************************
******************** RTClockx ****************************************
**********************************************************************
*/

void IRAM_ATTR  clockCB1sec(void) { CLOCK_TICK_1_SEC = true; }
        
RTClock*    initRTClock(void) {
    RTClock* rtc = new RTClock();
    rtc->init();
    rtc->startTicking();
    return rtc;
}

void RTClock::init(void) {
    _rtcSoft = new RTC_Millis();
    _rtcSoft->begin(DateTime(F(__DATE__),F(__TIME__)));

    _rtcHard = new RTC_DS3231();
    if (_rtcHard->begin()) {
        PL("Found ds3231 RTC");
        if (_rtcHard->lostPower()) {
            PL("RTC lost power, let's set the time!");
            // When time needs to be set on a new device, or after a power loss, the
            // following line sets the RTC to the date & time this sketch was compiled
            _rtcHard->adjust(DateTime(F(__DATE__), F(__TIME__)));
            // This line sets the RTC with an explicit date & time, for example to set
            // January 21, 2014 at 3am you would call:
            // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
        }
    } else {
        PL("Couldn't find ds3231 RTC");
        delete _rtcHard;
        _rtcHard = nullptr;

        scheduler = new Scheduler();
        scheduler->init();
    }
}

void RTClock::startTicking(void) {
    if (_rtcHard) {
        pinMode(RTC_SQW_PIN,INPUT);
        digitalWrite(RTC_SQW_PIN,HIGH);
        _rtcHard->writeSqwPinMode(DS3231_SquareWave1Hz);
        attachInterrupt(digitalPinToInterrupt(RTC_SQW_PIN), clockCB1sec, FALLING);
    } 
    else {
        scheduler->addTask(schedulerTask1sec);
        schedulerTask1sec.enable();
    }
}

void RTClock::adjust(const DateTime& dt) {
    _rtcSoft->adjust(dt);
    if (_rtcHard) 
        _rtcHard->adjust(dt);
}

DateTime RTClock::now(void) {
    return _rtcHard ? _rtcHard->now() : _rtcSoft->now();
}

void RTClock::tick(void) {
    if (scheduler) scheduler->execute();

}
/*
**********************************************************************
******************** RTTimer ****************************************
**********************************************************************
*/

void RTTimer::start(uint32_t currentMS, uint32_t intervalMS) {
    _start = currentMS;
    _interval = intervalMS;
    _count = 0;
}
void RTTimer::stop(void) { 
    _interval = 0;
    _count = 0;
}
bool RTTimer::tick(uint32_t current) {
    if (!_interval)
        return false;
    if ((current - _start) > _interval) {
        _start = current;
        _count++;
        return true;
    }
    return false;
}

uint32_t RTTimer::count(void) {
    return _count;
}
