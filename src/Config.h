#pragma once

class Config {

    public:
        void    init(void);

        char    _future[20];    // iso formate "2023-12-29T15:45:00"
        char    _message[13];
};

extern Config* initConfig(void);