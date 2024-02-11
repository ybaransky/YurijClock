#pragma once
#include <Arduino.h>

struct Digits {
    int  d1000; int  d100; int  d10; int  d1; // decimal of the n-th dogot
    char c1000; char c100; char c10; char c1; // character of the n-th digit

    Digits(int value=0);
    Digits(uint8_t value=0);
    void  set(int);
    bool  adjustTo12Hours(void); // true means its pm
};

