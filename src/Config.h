#pragma once

class Config {

    public:
        void    init(void);

        int     getMode(void);
        int     restoreMode(void);
        void    setMode(int mode);
        void    incMode(void);

        int     getFormat(void);
        int     getFormat(int);
        void    setFormat(int,int);
        void    incFormat(void);
        bool    isTenthSecFormat(void);

        uint8_t       getBrightness(void);
        void          setBrightness(uint8_t);

        const String& getMsgStart(void);
        void          setMsgStart(const String&);
        const String& getMsgEnd(void);
        void          setMsgEnd(const String&);

        const String& getAPName(void);
        void          setAPName(const String&);

        const String& getAPPassword(void);
        void          setAPPassword(const String&);

        void          saveFile(void);
        void          print(void) const;

        String  _isoFuture;                      // 20 chars iso format "2023-12-29T15:45:00"
        String  _msgStart;  /// at most 12 chars
        String  _msgEnd;
        int     _format, _formats[N_MODES];
        int     _mode, _prevMode;
        uint8_t _brightness;

        String  _apName;
        String  _apPassword;
        String  _filename;
};

extern Config*  initConfig(void);
extern Config*  config;