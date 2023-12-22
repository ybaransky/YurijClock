#pragma once
#include "Constants.h"

class Config {

    public:
        void            init(void);

        int             getMode(void);
        int             getNextMode(void);
        void            setMode(int mode);

        int             getFormat(void);
        int             getNextFormat(void);
        void            setFormat(int mode);
        bool            isTenthSecFormat(void);

        uint8_t         getBrightness(void);
        void            setBrightness(uint8_t);

        const String&   getFilename();
        void            saveFile(void);
        void            print(void) const;

        // when we are the access po
        String          _apSSID;
        String          _apPassword;

        // time string must be nn iso format (20 chars "2023-12-29T15:45:00")
        String          _timeStart;     // if we are counting up, this t=0 (in the past)
        String          _timeFinal;     // if we are counintg down, this is t=0 (in the future)
        
        // msg display, must be <= 12 chars
        String          _msgStart;      // msg to say at bootup.   
        String          _msgFinal;      // msg to say when countdown hits 0

        // what mode we are in (COUNTDOWN, COUNTUP, ..
        int             _mode;
        uint8_t         _brightness;
        
        // each mode has a set of formats
        int             _formats[N_MODES];

        // all segments have the same brightness
  //      uint8_t         _brightness;
};

extern Config*  initConfig(void);
extern Config*  config;