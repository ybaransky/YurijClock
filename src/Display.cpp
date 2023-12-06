#include <Arduino.h>
#include "Display.h"
#include "Constants.h"

void    Display::init(void) {
    _display[0] = new TM1637Display(TM1637_CLK_0, TM1637_DIO_0);
    _display[1] = new TM1637Display(TM1637_CLK_1, TM1637_DIO_1);
    _display[2] = new TM1637Display(TM1637_CLK_2, TM1637_DIO_2);
}