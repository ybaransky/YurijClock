#include "RTClock.h"
#include "Debug.h"
#include "Constants.h"

/*
**********************************************************************
******************** RTClockx ****************************************
**********************************************************************
*/

void IRAM_ATTR  clockCB1sec(void) { 
    extern volatile bool EVENT_CLOCK_1_SEC;
    EVENT_CLOCK_1_SEC = true;
}
        
RTClock*    initRTClock(void) {
    RTClock* rtc = new RTClock();
    rtc->init();
    return rtc;
}

void RTClock::init(void) {
    _lostPower = false;
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
            _lostPower = true;
        }
    } else {
        PL("Couldn't find ds3231 RTC");
        delete _rtcHard;
        _rtcHard = nullptr;
    }
}

bool RTClock::startTicking(void) {
    if (_rtcHard) {
        pinMode(RTC_SQW_PIN,INPUT);
        digitalWrite(RTC_SQW_PIN,HIGH);
        _rtcHard->writeSqwPinMode(DS3231_SquareWave1Hz);
        attachInterrupt(digitalPinToInterrupt(RTC_SQW_PIN), clockCB1sec, FALLING);
        return true;
    } 
    return false;
}

void RTClock::adjust(const DateTime& dt) {
    _rtcSoft->adjust(dt);
    if (_rtcHard) 
        _rtcHard->adjust(dt);
}

DateTime RTClock::now(void) {
    return _rtcHard ? _rtcHard->now() : _rtcSoft->now();
}

bool RTClock::lostPower(void) { return _lostPower; }
bool RTClock::noRealTime(void) { return _rtcHard == nullptr; }
