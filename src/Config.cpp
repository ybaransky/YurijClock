#include <Arduino.h>

#include "Config.h"
#include "Debug.h"

#define DEFAULT_FUTURE  "2023-12-22T15:45:00"
#define DEFAULT_MESSAGE "YuriCloc"

Config* initConfig(void) {
    Config* cfg = new Config();
    cfg->init();
    return cfg;
}

void    Config::init(void) {
    strncpy(_future, DEFAULT_FUTURE, N_ELETS(_future));
    strncpy(_message, DEFAULT_MESSAGE, N_ELETS(_message));
}