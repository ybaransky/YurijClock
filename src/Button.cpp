#include <Arduino.h>
#include "Button.h"
#include "RTClock.h"
#include "PinMap.h"
#include "Debug.h"

volatile bool   BUTTON_SINGLE_CLICK = false;
volatile bool   BUTTON_DOUBLE_CLICK = false;
volatile bool   BUTTON_LONG_CLICK   = false;

// push button
void oneButtonSingleClick() { BUTTON_SINGLE_CLICK = true; }
void oneButtonDoubleClick() { BUTTON_DOUBLE_CLICK = true; }
void oneButtonLongPress()   { BUTTON_LONG_CLICK = true; }
OneButton* initOneButton() {
    OneButton* btn = new OneButton(ONEBUTTON_PIN,false,false);  // D8
    btn->attachClick(oneButtonSingleClick);
    btn->attachDoubleClick(oneButtonDoubleClick);
    btn->attachLongPressStart(oneButtonLongPress);
    btn->setClickMs(400);
    return btn;
}
