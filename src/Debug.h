#pragma once
#include <Arduino.h>

#define P(x)            Serial.print(x);
#define PL(x)           Serial.println(x);
#define PV(x)           P(#x); P("="); P(x)
#define PVL(x)          P(#x); P("="); PL(x)
#define SP              P(" ")
#define EQ              P("=")


