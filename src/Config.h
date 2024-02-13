#pragma once
#include <ArduinoJson.h>
#include <RTClib.h>
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
        int             getHourFormat(void);
        void            setHourFormat(int hourFormat);
        bool            isTenthSecFormat(void);

        void            setTimeStart(const String&);    //iso format
        const String&   getTimeStart(void);
        const DateTime& getTimeStartDT(void);
        void            setTimeEnd(const String&);      //iso format
        const String&   getTimeEnd(void);
        const DateTime& getTimeEndDT(void);

        void            setMsgStart(const String&);    //iso format
        const String&   getMsgStart(void);
        void            setMsgEnd(const String&);      //iso format
        const String&   getMsgEnd(void);

        uint8_t         getBrightness(void);
        void            setBrightness(uint8_t);

        void            setSSID(const String&, const char* fcn=nullptr);
        const String&   getSSID(void);
        void            setPassword(const String&, const char* fcn=nullptr);
        const String&   getPassword(void);

        bool            loadFile(void);                 // loads file contents into config
        bool            saveFile(const char* fcn=nullptr) const;

        bool            fileToString(String& json) const;   // loads file contents into string, not config
        void            configToString(String& json) const;

        void            print(void) const;
        void            printFile(void) const;


        // when we are the access po
        String          _apSSID;
        String          _apPassword;

        // time string must be nn iso format (20 chars "2023-12-29T15:45:00")
        String          _timeStart;     // if we are counting up, this t=0 (in the past)
        DateTime        _timeStartDT;
        String          _timeEnd;     // if we are counintg down, this is t=0 (in the future)
        DateTime        _timeEndDT;
        
        // msg display, must be <= 12 chars
        String          _msgStart;      // msg to say at bootup.   
        String          _msgEnd;      // msg to say when countdown hits 0

        // what mode we are in (COUNTDOWN, COUNTUP, ..
        int             _mode;
        // each mode has a set of formats
        int             _formats[N_MODES];
        int             _hourFormat;    // 24 or 12 hour format

        // all segments have the same brightness
        uint8_t         _brightness;

    private:
        void    jsonToConfig(const JsonDocument&);
        void    configToJson(JsonDocument&) const;

        bool    fileToJson(File&, JsonDocument&) const;
        bool    jsonToFile(JsonDocument&, File&) const;

        File    getFileObject(const char* mode) const;
        void    printJsonMemoryStats(JsonDocument& doc) const;
};

extern Config*  initConfig(void);
extern Config*  config;