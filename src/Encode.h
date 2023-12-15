#pragma once
#include <Arduino.h>

extern uint8_t    encodeChar(uint8_t c);
extern uint8_t    encodeChar(char c);

extern uint8_t*	  reverse(uint8_t*);
extern char*	    reverse(char*);