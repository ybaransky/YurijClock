#include "Digits.h"

/*
*************************************************************************
*  Digit Class
*************************************************************************
*/

Digits::Digits(int value) { set(value); }

Digits::Digits(uint8_t value) { set(int(value)); }
void  Digits::set(int value) {
    d1    = value%10; value = value/10;
    d10   = value%10; value = value/10;
    d100  = value%10; value = value/10;
    d1000 = value%10; value = value/10;
    c1    = '0' + d1;
    c10   = '0' + d10;
    c100  = '0' + d100;
    c1000 = '0' + d1000;
}

