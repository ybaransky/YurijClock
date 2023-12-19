#pragma once

#include <OneButton.h>

extern volatile bool   BUTTON_SINGLE_CLICK;
extern volatile bool   BUTTON_DOUBLE_CLICK;
extern volatile bool   BUTTON_LONG_CLICK;

extern OneButton* initOneButton(void);