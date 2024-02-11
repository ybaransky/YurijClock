#include "Digits.h"

/*
*************************************************************************
*  Digit Class
*************************************************************************
*/

Digits::Digits(int value)     { set(value); }
Digits::Digits(uint8_t value) { set(int(value)); }

void  Digits::set(int value) {
    
    // the actual digits of this integer
    d1    = value%10; value = value/10;
    d10   = value%10; value = value/10;
    d100  = value%10; value = value/10;
    d1000 = value%10; value = value/10;
    
    //these are the char values
    c1    = '0' + d1;
    c10   = '0' + d10;
    c100  = '0' + d100;
    c1000 = '0' + d1000;
}

bool  Digits::adjustTo12Hours(void) {
    // if hours > 12, the subtract 12
    P("before: ");P(d10);SPACE;P(d1);P("    after: ");P(d10);SPACE;PL(d1);
    bool isPM = true;
    int hour = d1 + 10*d10;
    if (hour > 12) {
        d1  = hour-12;
        d10 = 0;

        c1  = '0' + d1;
        c10 = '0';
        isPM = true;
    }
    return isPM;
}
