#pragma once
#include <ArduinoJson.h>
#include "Constants.h"

class Config {

    public:
        void            init(void);

        int             getMode(void);
        int             getNextMode(void);
        void            setMode(int mode, const char* caller=nullptr);

        int             getFormat(void);       // gets current mode format
        int             getFormat(int mode);   // gets mode format
        int             getNextFormat(void);
        void            setFormat(int mode);
        void            setFormat(int format, int mode);
        bool            isTenthSecFormat(void);

        void            setTimeStart(const String&);    //iso format
        const String&   getTimeStart(void);
        void            setTimeEnd(const String&);      //iso format
        const String&   getTimeEnd(void);

        void            setMsgStart(const String&);    //iso format
        const String&   getMsgStart(void);
        void            setMsgEnd(const String&);      //iso format
        const String&   getMsgEnd(void);

        uint8_t         getBrightness(void);
        void            setBrightness(uint8_t);

        void            setSSID(const String&);
        const String&   getSSID(void);

        const String&   getFileName() const;
        void            saveFile(void) const;
        bool            loadFile(void);

        void            print(void) const;
        void            printJson(void) const;

        // when we are the access po
        String          _apSSID;
        String          _apPassword;

        // time string must be nn iso format (20 chars "2023-12-29T15:45:00")
        String          _timeStart;     // if we are counting up, this t=0 (in the past)
        String          _timeEnd;     // if we are counintg down, this is t=0 (in the future)
        
        // msg display, must be <= 12 chars
        String          _msgStart;      // msg to say at bootup.   
        String          _msgEnd;      // msg to say when countdown hits 0

        // what mode we are in (COUNTDOWN, COUNTUP, ..
        int             _mode;
        // each mode has a set of formats
        int             _formats[N_MODES];

        // all segments have the same brightness
        uint8_t         _brightness;
    private:
        bool      serialize(Print& dst) const;
        void      saveToJson(JsonDocument& doc) const;
        void      loadFromJson(const JsonDocument& doc);
};

extern Config*  initConfig(void);
extern Config*  config;