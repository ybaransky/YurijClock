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
  " YYYY | MM:DD | hh:mm", // 0 // blinking determined by secsMode
  " YYYY |    MM |    DD", // 1
  "   MM |    DD | hh:mm", // 2  // blinking determined by secsMode
  "MM:DD | hh:mm | ss  u", // 3
  "MM:DD | hh:mm |    ss", // 4
  "MM:DD |    hh | mm:ss", // 5
  "MM:DD |    hh |    mm", // 6
  "   DD | hh:mm | ss  u", // 7
  "   DD | hh:mm |    ss", // 8
  "   DD |    hh | mm:ss", // 9
  "   DD |    hh |    mm", // 0
};

const char** formatNames[N_MODES] = {formatNamesCountDown, formatNamesCountUp, formatNamesClock};

const char* modeNames[N_MODES] = {
  "Countdown", 
  "Countup", 
  "Clock", 
};

const char* hourModeNames[N_HOUR_MODES]  = {"24hr", "12hr"};
const int   hourModeValues[N_HOUR_MODES] = {0,1};

const char* secsModeNames[N_SECS_MODES]  = {"blink", "no blink"};
const int   secsModeValues[N_SECS_MODES] = {0,1};