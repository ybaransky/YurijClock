#include "Constants.h"

#ifdef YURIJ
const char *formatNames[] = {
  (*char[]) {
    "dd D | hh:mm |  ss.u",
    "dd D | hh:mm |    ss",
    "dd D | hh  H | mm:ss",
    "dd D | hh  H |  mm N",
    "  dd | hh:mm |  ss.u",
    "  dd | hh:mm |    ss",
    "  dd |    hh | mm:ss",
    "  dd |    hh |    mm"
  },
  (*char[]) {
    "dd D | hh:mm |  ss.u",
    "dd D | hh:mm |    ss",
    "dd D | hh  H | mm:ss",
    "dd D | hh  H |  mm N",
    "  dd | hh:mm |  ss.u",
    "  dd | hh:mm |    ss",
    "  dd |    hh | mm:ss",
    "  dd |    hh |    mm",
  },
  (*char[]) {
    " YYYY | MM:DD | hh:mm",  // blinking hh:mm colon
    " YYYY | MM:DD | hh:mm",
    " YYYY |    MM |    DD",
    "   MM |    DD | hh:mm",  // blinking hh:mm colon
    "   MM |    DD | hh:mm",
    "MM:DD | hh:mm | ss  u",
    "MM:DD | hh:mm |    ss",
    "MM:DD |    hh | mm:ss",
    "MM:DD |    hh |    mm",
    "   DD | hh:mm | ss  u",
    "   DD | hh:mm |    ss",
    "   DD |    hh | mm:ss",
    "   DD |    hh |    mm",
  },
  (*char[]) {
    " Left justified",
    "Right justified"
  }
};
#endif

const char* formatNamesCountDown[N_FORMAT_COUNTDOWN] = {
  "dd D | hh:mm |  ss.u",
  "dd D | hh:mm |    ss",
  "dd D | hh  H | mm:ss",
  "dd D | hh  H |  mm N",
  "  dd | hh:mm |  ss.u",
  "  dd | hh:mm |    ss",
  "  dd |    hh | mm:ss",
  "  dd |    hh |    mm"
};

const char* formatNamesCountUp[N_FORMAT_COUNTUP] = {
  "dd D | hh:mm |  ss.u",
  "dd D | hh:mm |    ss",
  "dd D | hh  H | mm:ss",
  "dd D | hh  H |  mm N",
  "  dd | hh:mm |  ss.u",
  "  dd | hh:mm |    ss",
  "  dd |    hh | mm:ss",
  "  dd |    hh |    mm"
}; 

const char* formatNamesClock[N_FORMAT_CLOCK] = {
  " YYYY | MM:DD | hh:mm",  // blinking hh:mm colon
  " YYYY | MM:DD | hh:mm",
  " YYYY |    MM |    DD",
  "   MM |    DD | hh:mm",  // blinking hh:mm colon
  "   MM |    DD | hh:mm",
  "MM:DD | hh:mm | ss  u",
  "MM:DD | hh:mm |    ss",
  "MM:DD |    hh | mm:ss",
  "MM:DD |    hh |    mm",
  "   DD | hh:mm | ss  u",
  "   DD | hh:mm |    ss",
  "   DD |    hh | mm:ss",
  "   DD |    hh |    mm",
};

const char* formatNamesText[N_FORMAT_CLOCK] = {
  " Left justified",
  "Right justified"
};

const char** formatNames[N_MODES] = {formatNamesCountDown, formatNamesCountUp, formatNamesClock, formatNamesText};

const char* modeNames[N_MODES] = {
  "Countdown", 
  "Countup", 
  "Clock", 
  "Text", 
};
