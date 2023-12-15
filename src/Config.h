#pragma once

class Config {

    public:
        void    init(void);

        int     getMode(void);
        int     restoreMode(void);
        void    setMode(int mode);
        void    incMode(void);

        int     getFormat(void);
        void    setFormat(int);
        void    incFormat(void);

        uint8_t getBrightness(void);
        void    setBrightness(uint8_t);

        void    setText(const String&);
        String& getText(void);

        void    print(void) const;

        String  _future;    // 20 chars iso formate "2023-12-29T15:45:00"
        String  _text;      // 12 chars + null
        int     _formats[N_MODES];
        int     _mode,_prevMode;
        uint8_t _brightness;
};

extern Config*  initConfig(void);
extern Config*  config;