// #include <TaskScheduler.h>
#include "Debug.h"
#include "Constants.h"
#include "RTCClock.h"
        
void RTC::init(void) {
    _rtcSoft = nullptr;
    _rtcHard = new RTC_DS3231();
    if (!_rtcHard->begin()) {
        PL("Couldn't find ds3231 RTC");
        // use the mc's internal clock 
        delete _rtcHard;
        _rtcHard = nullptr;
        _rtcSoft = new RTC_Millis();
        _rtcSoft->begin(DateTime(F(__DATE__),F(__TIME__)));
    } else {
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
    }
}


DateTime RTC::now(void) {
    return _rtcHard ? _rtcHard->now() : _rtcSoft->now();
}

bool RTC::attachSQWInterrupt(void (*callback)(void)) {
    if (_rtcHard) {
        _rtcHard->writeSqwPinMode(DS3231_SquareWave1Hz);
        pinMode(RTC_SQW_PIN,INPUT);
        digitalWrite(RTC_SQW_PIN,HIGH);
        attachInterrupt(digitalPinToInterrupt(RTC_SQW_PIN), callback, FALLING);
        return true;
    } 
    return false;
}