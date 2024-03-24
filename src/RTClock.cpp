#include "RTClock.h"
#include "Debug.h"
#include "PinMap.h"

/*
**********************************************************************
******************** Clockx ****************************************
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
    _rtcHard = new RTC_DS3231();
    _rtcSoft = nullptr;

    if (_rtcHard->begin()) {
        PL("Found ds3231 RTC");
        if (_rtcHard->lostPower()) {
            _lostPower = true;
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
        _rtcSoft = new RTC_Millis();
    }
}

bool RTClock::start(void) {
    if (hasDevice()) {
        pinMode(RTC_SQW_PIN,INPUT);
        digitalWrite(RTC_SQW_PIN,HIGH);
        _rtcHard->writeSqwPinMode(DS3231_SquareWave1Hz);
        attachInterrupt(digitalPinToInterrupt(RTC_SQW_PIN), clockCB1sec, FALLING);
        return true;
    } else {
        _rtcSoft->begin(DateTime(F(__DATE__),F(__TIME__)));
    }
    return false;
}

void RTClock::adjust(const DateTime& dt) {
    P("RTClock::adjust "); PL(dt.timestamp());
    if (hasDevice()) 
        _rtcHard->adjust(dt);
    else
        _rtcSoft->adjust(dt);
}

DateTime RTClock::now(void) {
    return hasDevice() ? _rtcHard->now() : _rtcSoft->now();
}

bool RTClock::lostPower(void) { return _lostPower; }
bool RTClock::hasDevice(void) { return _rtcHard != nullptr; }
