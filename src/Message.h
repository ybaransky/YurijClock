#pragma once
#include <Arduino.h>

class Message {
  public:
      void          set(const String&  msg, bool blink=false);
      const bool&   isBlinking(void) const { return _blink;}
      const String& text(void) const { return _text; }
      void          print(void) const;
  private:
      String  _text;
      bool    _blink;
};