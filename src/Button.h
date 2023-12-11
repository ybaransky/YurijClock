#pragma once

#include <OneButton.h>

extern volatile bool   SINGLE_BUTTON_CLICK;
extern volatile bool   DOUBLE_BUTTON_CLICK;
extern volatile bool   LONG_BUTTON_CLICK;

extern OneButton* initOneButton(void);