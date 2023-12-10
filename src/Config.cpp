#include <Arduino.h>

#include "Config.h"
#include "Debug.h"

#define DEFAULT_FUTURE  "2023-12-29T15:45:00"

Config* initConfig(void) {
    Config* cfg = new Config();
    cfg->init();
    return cfg;
}

void    Config::init(void) {
    strncpy(_future, DEFAULT_FUTURE, N_ELETS(_future));
}