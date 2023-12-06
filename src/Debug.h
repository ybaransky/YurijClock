#pragma once
#include <Arduino.h>

#define P(x)            Serial.print(x)
#define PL(x)           Serial.println(x)
#define PV(x)           P(#x); P("="); P(x)
#define PVL(x)          P(#x); P("="); PL(x)
#define SPACE           P(" ")

#define NELEMS(x)       (sizeof(x) / sizeof((x)[0])     // number of elements in array x

