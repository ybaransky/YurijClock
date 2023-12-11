#include <Arduino.h>
#include "Display.h"
#include "Constants.h"
#include "Debug.h"
/*
12/7/2023
For some reason, I can't create the TM1637Display objects via new. 
This needs to be understood 
*/
/*
*************************************************************************
*  Device Class
*************************************************************************
*/

Display* initDisplay(void) {
  Display* display = new Display();
  display->init();
  return display;
}

void Display::init(void) {

  for(int i=0; i < N_SEGMENTS; i++) {
    _segments[i].init(i);
    _segments[i].device().clear();
  }
}

void  Display::setBrightness(uint8_t brightness, bool on) {
  for(int i=0;i<N_SEGMENTS;i++) 
    _segments[i].device().setBrightness(brightness, on);
}

void  Display::setFormat(int format) {
    if (_format == format) return;
    _format = format;
    for(int i=0;i<3;i++) _segments[i].setFormat(format);
    refresh(_cache._ts, _cache._ms100);
}

void Display::test(void) {
  int values[] = {0000,1111,2222};
  bool zeroPad = true;
  bool on=true;
  for(int k=0; k < 2; k++) {
    for(int i=0;i<N_SEGMENTS;i++) {
      _segments[i].device().setBrightness(SEGMENT_BRIGHTEST,on);
      _segments[i].device().showNumberDecEx(values[i], SEGMENT_COLON, zeroPad);
    }
    delay(250);
    on = !on;
  }
  setBrightness(BRIGHTEST);
}

void Display::showInteger(int32_t ival) {
    int parts[3];
    parts[2] =  ival / 100000000;
    parts[1] = (ival % 100000000) / 10000;
    parts[0] = (ival % 10000);
    for(int i=0; i<3; i++)
      _segments[i].device().showNumberDec(parts[i],false);
}

void Display::showTimeSpan(TimeSpan ts, uint8_t ms100) {
  // for future use
  _cache.save(ts,ms100);
  refresh(ts,ms100);
  _segments[DDDD].drawDDDD(ts.days());
  _segments[HHMM].drawHHMM(ts.hours(),ts.minutes());
  _segments[SSUU].drawSSUU(ts.minutes(), ts.seconds(),ms100);
}

void Display::refresh(TimeSpan ts, uint8_t ms100) {
  _segments[DDDD].drawDDDD(ts.days());
  _segments[HHMM].drawHHMM(ts.hours(),ts.minutes());
  _segments[SSUU].drawSSUU(ts.minutes(), ts.seconds(),ms100);
}
#ifdef COUNTDOWN_DISPLAY
/*
 * ******************************************************************
 */

void  CountdownDisplay::draw_blank(int seg) {
  draw_c(seg, 3, ' ', false);
  draw_c(seg, 2, ' ', false);
  draw_c(seg, 1, ' ', false);
  draw_c(seg, 0, ' ', false);
  draw_colon(seg, false);
}

void  CountdownDisplay::draw_dddd(
    int dd1000, int dd100, int dd10, int dd1) {
  /*
     display modes
   "dd D | hh:mm |  ss.u",
   "dd D | hh:mm |    ss",
   "dd D | hh  H | mm:ss",
   "dd D | hh  H |  mm N",
   "  dd | hh:mm |  ss.u",
   "  dd | hh:mm |    ss",
   "  dd |    hh | mm:ss",
   "  dd |    hh |    mm"
   */
  bool  bdd1000 = bool(dd1000);
  bool  bdd100  = bool(dd100);
  bool  bdd10   = bool(dd10);
  bool  bdd1    = bool(dd1); 
  int   seg = DDDD;
  if (bdd1000 || bdd100) {
    // dddd | .... | ....
    draw_x(seg, 3, dd1000, bdd1000 );
    draw_x(seg, 2,  dd100, bdd1000 || bdd100 );
    draw_x(seg, 1,   dd10, bdd1000 || bdd100 || bdd10 );
    draw_x(seg, 0,    dd1, bdd1000 || bdd100 || bdd10 || bdd1 );
  } else {
    if (gConfig._display_mode < 4) {
        // dd D | .... | ....
        draw_x(seg, 3, dd10, bdd10 );
        draw_x(seg, 2,  dd1, bdd10 || bdd1 );
        draw_c(seg, 1,  ' ', false );
        draw_c(seg, 0,  'd', bdd10 || bdd1 );
    }  else {
      //   dd | .... | ....
      draw_c(seg, 3,  ' ', false );
      draw_c(seg, 2,  ' ', false );
      draw_x(seg, 1, dd10, bdd10 );
      draw_x(seg, 0,  dd1, bdd10 || bdd1 );
    }
  }
  draw_colon(seg, false);
}

void  CountdownDisplay::draw_hhmm(
    int hh10, int hh1, int mm10, int mm1, bool bdays) {
  /*
     display modes
   "dd D | hh:mm |  ss.u",
   "dd D | hh:mm |    ss",
   "dd D | hh  H | mm:ss",
   "dd D | hh  H |  mm N",
   "  dd | hh:mm |  ss.u",
   "  dd | hh:mm |    ss",
   "  dd |    hh | mm:ss",
   "  dd |    hh |    mm"
   */
  bool  bhh10 = bool(hh10);
  bool  bmm10 = bool(mm10);
  bool  bhh1  = bool(hh1);
  bool  bmm1  = bool(mm1);
  bool  bhours   = bhh10 || bhh1;
  bool  bminutes = bmm10 || bmm1;
  int   seg = HHMM;
  _segments[seg]._changed = false;
  switch (gConfig._display_mode) {
    case 0:
    case 1:
    case 4:
    case 5:
      //  .... | hh:mm | ....
      draw_x(seg, 3, hh10, bhh10);
      draw_x(seg, 2, hh1,  bdays || bhours || bminutes);
      draw_x(seg, 1, mm10, bdays || bhours || bminutes);
      draw_x(seg, 0, mm1,  bdays || bhours || bminutes);
      draw_colon(seg, bdays || bhours || bminutes);
      break;
    case 2:
    case 3:
      //  .... | hh H | ....
      draw_x(seg, 3, hh10, bhh10);
      draw_x(seg, 2, hh1,  bdays || bhours);
      draw_c(seg, 1,  ' ', false );
      draw_c(seg, 0, 'h',  bdays || bhours);
      draw_colon(seg, false);
      break;
    case 6:
    case 7:
    defaut :
      //  .... |  hh | ....
      draw_c(seg, 3,  ' ', false );
      draw_c(seg, 2,  ' ', false );
      draw_x(seg, 1, hh10, bhh10);
      draw_x(seg, 0, hh1,  bdays || bhours);
      draw_colon(seg, false);
  }
}

void  CountdownDisplay::draw_ssuu(
    int mm10, int mm1, int ss10, int ss1, int uu100, bool bdays, bool bhours) {
  /*
     display modes
   "dd D | hh:mm |  ss.u",
   "dd D | hh:mm |    ss",
   "dd D | hh  H | mm:ss",
   "dd D | hh  H |  mm N",
   "  dd | hh:mm |  ss.u",
   "  dd | hh:mm |    ss",
   "  dd |    hh | mm:ss",
   "  dd |    hh |    mm"
   */
  bool  buu100 = bool(uu100);
  bool  bmm10  = bool(mm10);
  bool  bss10  = bool(ss10);
  bool  bmm1   = bool(mm1);
  bool  bss1   = bool(ss1);
  bool  bminutes = bmm10 || bmm1;
  bool  bseconds = bss10 || bss1;
  int seg = SSUU;
  _segments[seg]._changed = false;
  switch (gConfig._display_mode) {
    case 0:
    case 4:
      //  .... | .... | ss.u",
      draw_c(seg, 3,  ' ', false );
      draw_x(seg, 2, ss10, bss10);
      draw_x(seg, 1, ss1,   true, true); // has the decimal
      draw_x(seg, 0, uu100, true);
      draw_colon(seg, false);
      break;
    case 1:
    case 5:
      //  .... | .... |  ss",
      draw_c(seg, 3,  ' ', false );
      draw_c(seg, 2,  ' ', false );
      draw_x(seg, 1, ss10, bss10);
      draw_x(seg, 0, ss1,   true); 
      draw_colon(seg, false);
      break;
    case 2:
    case 6:
      //  .... | .... | mm:ss",
      draw_x(seg, 3, mm10, bmm10);
      draw_x(seg, 2, mm1,  bdays || bhours || bminutes);
      draw_x(seg, 1, ss10, bdays || bhours || bminutes || bss10);
      draw_x(seg, 0, ss1,  bdays || bhours || bminutes || bseconds);
      draw_colon(seg, bdays || bhours || bminutes);
      break;
    case 3:
      //  .... | .... | mm N",
      draw_x(seg, 3, mm10, bmm10);
      draw_x(seg, 2, mm1,  bdays || bhours || bminutes || bseconds, uu100 < 5);
      draw_c(seg, 1, ' ',  false );
      draw_c(seg, 0, 'n',  bdays || bhours || bminutes || bseconds);
      draw_colon(seg, false);
      break;
    case 7:
    default :
      //  .... | .... | mm",
      draw_c(seg, 3,  ' ', false );
      draw_c(seg, 2,  ' ', false );
      draw_x(seg, 1, mm10, bmm10);
      draw_x(seg, 0, mm1,  bdays || bhours || bminutes || bseconds, uu100 < 5);
      draw_colon(seg, false);
      break;
  }
}

void  CountdownDisplay::draw_colon(int seg, bool visible) {
  _segments[seg].device().drawColon(visible);
  if(_segments[seg].prev().changed(visible))
    _segments[seg]._changed = true;
  _colons[seg] = visible;
}

void  CountdownDisplay::draw_c(int seg, int i, char c, bool visible) {
  int offset;
  switch(seg) {
    case DDDD : offset = 8; break;
    case HHMM : offset = 4; break;
    default   : offset = 0; break;
  }
  c = visible ? c : ' ';
  _segments[seg].device().writeDigitRaw(_imap[i], asci_mask(c));
  if (_segments[seg].prev().changed(i,c,false))
    _segments[seg]._changed = true;
  _buffer[i + offset] = c;
}

void  CountdownDisplay::draw_x(int seg, int i, int n, bool visible, bool decimal) {
  int offset;
  switch(seg) {
    case DDDD : offset = 8; break;
    case HHMM : offset = 4; break;
    default   : offset = 0; break;
  }

  char c;
  if (visible) {
    c = '0' + n;
    _segments[seg].device().writeDigitNum(_imap[i], n, decimal);
  } else {
    c = ' ';
    _segments[seg].device().writeDigitRaw(_imap[i], asci_mask(c));
  }
  if (_segments[seg].prev().changed(i,c,decimal))
    _segments[seg]._changed = true;
  _buffer[i + offset] = c;
}

void  CountdownDisplay::displayTime(int* time) {
  int   ones,tens,bundreds,thousands;
  int   uu100  = time[MILLIS]/100;
  int   ss10   = time[SECONDS]/10;
  int   ss1    = time[SECONDS]%10;
  int   mm10   = time[MINUTES]/10;
  int   mm1    = time[MINUTES]%10;
  int   hh10   = time[HOURS]/10;
  int   hh1    = time[HOURS]%10;
  int   dd1000 = (time[DAYS]/1000);
  int   dd100  = (time[DAYS]%1000)/100;
  int   dd10   = (time[DAYS]%100)/10;
  int   dd1    = (time[DAYS]%10);
  int   seg;

  /*
     display modes
   "dd D | hh:mm |  ss.u",
   "dd D | hh:mm |    ss",
   "dd D | hh  H | mm:ss",
   "dd D | hh  H |  mm N",
   "  dd | hh:mm |  ss.u",
   "  dd | hh:mm |    ss",
   "  dd |    hh | mm:ss",
   "  dd |    hh |    mm"
   */

  // DDDD segement
  seg = DDDD;
  _segments[seg]._changed = false;
  if (gConfig._segments[seg]._visible)
    draw_dddd(dd1000, dd100, dd10, dd1);
  else
    draw_blank(seg);

  // HHMM segmeent
  seg = HHMM;
  _segments[seg]._changed = false;
  if (gConfig._segments[seg]._visible)
    draw_hhmm(hh10, hh1, mm10, mm1, time[DAYS]);
  else
    draw_blank(seg);

  // SSUU segmeent
  seg = SSUU;
  _segments[seg]._changed = false;
  if (gConfig._segments[seg]._visible)
    draw_ssuu(mm10, mm1, ss10, ss1, uu100, time[DAYS], time[HOURS]);
  else
    draw_blank(seg);

  writeDisplay();
}

void  CountdownDisplay::print(const char* msg, bool text ) {
  if (msg)
    Serial.printf("%s ",msg);

  bool  hasDecimal = 
    (gConfig._display_mode==0) || 
    (gConfig._display_mode==4);

  if (text)
    hasDecimal = false;

  char *b = _buffer;
  int loop[] = {DDDD,HHMM,SSUU};
  Serial.print("'");
  for (int i=0; i < N_SEGMENTS; i++) {
    int seg = loop[i];
    int k=4*seg;

    if (_colons[seg]) {
      if (hasDecimal) {
        Serial.printf("%c%c:%c.%c",b[3+k],b[2+k],b[1+k],b[0+k]);
      } else {
        Serial.printf("%c%c:%c%c", b[3+k],b[2+k],b[1+k],b[0+k]);
      }
    } else {
      if (hasDecimal) {
        Serial.printf("%c%c%c.%c", b[3+k],b[2+k],b[1+k],b[0+k]);
      } else {
        Serial.printf("%c%c%c%c",  b[3+k],b[2+k],b[1+k],b[0+k]);
      }
    }

    if (i!=N_SEGMENTS-1)
      Serial.printf("|");
  }
  Serial.printf("'\n");
}

////////////////////////////////////////////////////////////////////////
//
CountdownDisplay::CountdownDisplay(void) {
  // the 7 segement display is indexed as 0 is leftmost, i.e.
  // 0 1 2 3 4
  // _ _ : _ _
  //
  _imap[0] = 4;
  _imap[1] = 3;
  _imap[2] = 1;
  _imap[3] = 0;
}

void CountdownDisplay::begin(void) {
#ifdef OLD_STYLE
#ifdef WORK_CLOCK
  _segments[DDDD]._hardware.begin(0x72);
  _segments[HHMM]._hardware.begin(0x70);
  _segments[SSUU]._hardware.begin(0x71);
#else
  _segments[DDDD]._hardware.begin(0x72);
  _segments[HHMM]._hardware.begin(0x71);
  _segments[SSUU]._hardware.begin(0x70);
#endif
#endif
  for(int i=0;i<N_SEGMENTS;i++) {
    int addr = 0x70 + gConfig._segments[i]._address;
    _segments[i].device().begin(addr);
    _segments[i].prev().clear();
    _segments[i]._changed = true;
  }
  set_brightness();
}

void    CountdownDisplay::set_message(const String& message) {_message = message;}
String& CountdownDisplay::get_message(void) { return _message;}

void  CountdownDisplay::clear(bool write) {
  for(int i=0; i<N_DIGITS; i++) {
    _buffer[i] = ' ';
  }
  for(int i=0; i<N_SEGMENTS;i++) {
    _colons[i] = false;
    _segments[i].device().clear();
    _segments[i].prev().clear();
    _segments[i]._changed = true;
  }
  if (write)
    writeDisplay();
}

void  CountdownDisplay::writeDisplay(bool force) {
  for(int i=0; i<N_SEGMENTS;i++) {
    if (force || _segments[i]._changed)
      _segments[i].device().writeDisplay();
  }
}

void  CountdownDisplay::set_brightness() {
  for(int i=0;i<N_SEGMENTS;i++) {
    int b = gConfig._segments[i]._brightness;
    if (b <  0) b = 0;
    if (b > 15) b = 15;
    _segments[i].device().setBrightness(uint8_t(b));
  }
}

void  CountdownDisplay::displayBlinkMessage() {
  static  uint32_t  blinktime = millis();
  static  bool      display = false;
  uint32_t  currtime = millis();
  if (currtime - blinktime > 500) {
    blinktime = currtime;
    display  = !display;
    if (display)
      displayMessage();
    else
      clear();
  }
}

void  CountdownDisplay::displayMessage(void) {
  // the segment is designed with this inde
  // 0 1 : 3 4
  // the buffer starts from right, i.e.
  //   h e l l o   w o r l d
  // 1 0 9 8 7 6 5 4 3 2 1 0
  //              

  //
  // space pad thie to 12 digits
  char  buffer[N_DIGITS+1];
  sprintf(buffer,"%12s",_message.c_str());

  /*
  Serial.println("");
  Serial.printf("|109876543210|\n");
  Serial.printf("|%s|\n",buffer);
  */
  clear();

  int seg;
  seg = DDDD;
  if (gConfig._segments[seg]._visible) {
    draw_c(seg, 3, buffer[0], true);
    draw_c(seg, 2, buffer[1], true);
    draw_c(seg, 1, buffer[2], true);
    draw_c(seg, 0, buffer[3], true);
  } else {
    draw_blank(seg);
  }

  seg = HHMM;
  if (gConfig._segments[seg]._visible) {
    draw_c(seg, 3, buffer[4], true);
    draw_c(seg, 2, buffer[5], true);
    draw_c(seg, 1, buffer[6], true);
    draw_c(seg, 0, buffer[7], true);
  } else {
    draw_blank(seg);
  }

  seg = SSUU;
  if (gConfig._segments[seg]._visible) {
    draw_c(seg, 3, buffer[8], true);
    draw_c(seg, 2, buffer[9], true);
    draw_c(seg, 1, buffer[10], true);
    draw_c(seg, 0, buffer[11], true);
  } else {
    draw_blank(seg);
  }

  writeDisplay();
}
#endif