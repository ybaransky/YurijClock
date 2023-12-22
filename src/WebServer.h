#pragma once
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>

typedef ESP8266WebServer WebServer;

WebServer* initWebServer(void);

extern  WebServer*  server;