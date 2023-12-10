#include <Arduino.h>
#include "Button.h"
#include "RTClock.h"
#include "Constants.h"
#include "Debug.h"

// push button
void oneButtonSingleClick() { PV(millis()); SPACE; PL("singleClick"); }
void oneButtonDoubleClick() { PV(millis()); SPACE; PL("doubleClick"); }
void oneButtonLongPress()   { 
    extern RTClock* rtClock;
    DateTime dt(F(__DATE__),F(__TIME__));
    PV(millis()); SPACE; P("longPress"); 
    P(" adjusting Datetime to: "); PL(dt.timestamp(DateTime::TIMESTAMP_FULL));
    rtClock->adjust(dt);
}

OneButton* initOneButton() {
    OneButton* btn = new OneButton(ONEBUTTON_PIN,false,false);  // D8
    btn->attachClick(oneButtonSingleClick);
    btn->attachDoubleClick(oneButtonDoubleClick);
    btn->attachLongPressStart(oneButtonLongPress);
    btn->setClickMs(400);
    return btn;
}


