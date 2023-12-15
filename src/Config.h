#pragma once

class Config {

    public:
        void    init(void);

        int     getMode(void);
        int     restoreMode(void);
        void    setMode(int mode);

        int     getFormat(void);
        void    setFormat(int);
        void    incFormat(void);

        uint8_t getBrightness(void);
        void    setBrightness(uint8_t);

        void    print(void) const;

        char    _future[20];    // iso formate "2023-12-29T15:45:00"
        char    _message[13];
        int     _formats[N_MODES];
        int     _mode,_prevMode;
        uint8_t _brightness;
};

extern Config*  initConfig(void);
extern Config*  config;