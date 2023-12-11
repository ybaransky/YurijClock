#include <Arduino.h>
#include "Button.h"
#include "RTClock.h"
#include "Constants.h"
#include "Debug.h"


volatile bool   SINGLE_BUTTON_CLICK = false;
volatile bool   DOUBLE_BUTTON_CLICK = false;
volatile bool   LONG_BUTTON_CLICK   = false;

// push button
void oneButtonSingleClick() { SINGLE_BUTTON_CLICK = true; }
void oneButtonDoubleClick() { DOUBLE_BUTTON_CLICK = true; }
void oneButtonLongPress()   { LONG_BUTTON_CLICK = true; }
OneButton* initOneButton() {
    OneButton* btn = new OneButton(ONEBUTTON_PIN,false,false);  // D8
    btn->attachClick(oneButtonSingleClick);
    btn->attachDoubleClick(oneButtonDoubleClick);
    btn->attachLongPressStart(oneButtonLongPress);
    btn->setClickMs(400);
    return btn;
}


