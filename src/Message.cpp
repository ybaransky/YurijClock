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
}

void  Message::print(const char* msg) const {
  if (msg) {
    P(msg); SPACE;
  }
  P("Message:");
  SPACE; PV(_text);
  SPACE; PV(_blink);
  PL("");
}

