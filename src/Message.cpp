#include "Message.h"
#include "Debug.h"

/*
*************************************************************************
*  DisplayMessage Class
*************************************************************************
*/

void Message::set(const String& text, bool blink) {
  _text  = text;
  _blink = blink;
  P("DisplayMsg: |"); P(_text); PL("|");
}

void  Message::print(void) const {
  P("DMsg:");
  SPACE; PV(_text);
  SPACE; PV(_blink);
  PL("");
}

