#include <Arduino.h>
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include "Constants.h"
#include "Config.h"
#include "Display.h"
#include "RTClock.h"

bool  EVENT_DEMO_MODE = false;

extern Display    *display;
extern RTClock    *rtClock;
extern Config     *config;
extern WebServer  *server;

static  const String     NL("\r\n");
static  const String     EMPTY("");

static  const char STYLE_BUTTON[] PROGMEM = R"(
button {border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:40%;} )";

static  const char  STYLE_HEAD[] PROGMEM = R"(
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'>
  <title>Countdown Setup</title>
  <style type='text/css'>
    table       {text-align:center; border-collapse:collapse; width=90%; margin=0px auto;}
    th,td       {padding-left: 5px; padding-right: 5px; border:1px solid blue}
    td.noborder {border: 0px;}
    td.bold     {font-weight:bold;}
    td.left     {text-align:left;}
    td.center   {text-align:center;}
    td.right    {text-align:right;}
    td.grey     {background-color:#f2f2f2;}
    caption     {font-weight:bold;}
    button      {border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:40%;}
    input[type='text'], input[type='number'] {font-size:100%; border:2px solid red}
  </style>
</head>
)";

static
String inputFieldText(const char* id, const String& text, int size=0) 
{
  String input = NL;
  String sid(id);
  String ssize(size);
  input += "<input ";
  input += "type='text' ";
  input += "id='"          + sid   + "' ";
  input += "name='"        + sid   + "' ";
  input += "placeholder='" + text  + "' ";
  if (size) {
    input += "maxlength="    + ssize + " ";
    input += "size="         + ssize + " ";
  }
  input += ">";
  return input;
}

static 
String inputFieldComboBox(const char* id, const char* choices[], int ichoice, int n) {
  String input = NL;
  if (ichoice <  0) ichoice = 0;
  if (ichoice >= n) ichoice = 0;
//  input += "<div style='height:100%'>";
  input += "<select name='" + String(id) + "'>";
  for(int i=0;i<n;i++) {
    input += "<option value='" + String(i) + "'";
    if (i==ichoice)
      input += " selected";
    input += ">" + String(choices[i]) + "</option>";
  }
//  input += "</div>";
  return input;
}

static
String inputFieldDateTime(const char* id, const String& text)
{
  String input = NL;
  String sid(id);
  input += "<input ";
  input += "type='datetime-local' ";
  input += "id='"          + sid   + "' ";
  input += "name='"        + sid   + "' ";
  input += "value='"       + text  + "' ";
  input += "min='2000-06-07T00:00' max='2035-06-14T00:00'";
  input += ">";
  return input;
}

static 
String addInputRow(const char* desc, const String& value) {
  String row("");
  row += "<tr>";
  row += "<td class='right border grey'>" + String(desc) + "</td><td class='left border'>" + value + "</td>";
  row += "</tr>" + NL;
  return row;
}


//"th,td   {width:33%; padding-left: 5px; padding-right: 5px; border:1px solid blue}\r\n" 
//
static 
String configPageRow(const char* desc, const String& value) {
  String row("");
  row += "<tr>";
  //row += "<td style='text-align:right; font-weight:bold'>" + String(name) + "</td><td style='text-align:left'>" + value + "</td>";
  row += "<td class='right bold'>" + String(desc) + "</td><td class='left grey'>" + value + "</td>";
  row += "</tr>" + NL;
  return row;
}

static 
String configPageInputRow(const char* desc, const String& value) {
  String row("");
  row += "<tr>";
  row += "<td class='right border grey'>" + String(desc) + "</td><td class='left border'>" + value + "</td>";
  row += "</tr>" + NL;
  return row;
}

static 
String configPageInputRow3(const char* desc, const String& value) {
  String row("");
  row += "<tr>";
  row += "<td class='right border grey'>" + String(desc) + "</td>";
  row += "<td colspan='3' class='center border'>" + value + "</td>";
  row += "</tr>" + NL;
  return row;
}

static 
String configPageInputRow(const char* desc, const String& v1, const String& v2) {
  String row("");
  row += "<tr>";
  row += "<td class='right border grey'>" + String(desc) + "</td>";
  row += "<td class='center border'>" + v1 + "</td>";
  row += "<td class='left border'>" + v2 + "</td>";
  row += "</tr>" + NL;
  return row;
}

static 
String configPageInputRow2Cols(const char* desc, 
    const String& v1, const String& v2) {
  String row("");
  row += "<tr>";
  row += "<td class='right border grey'>" + String(desc) + "</td>";
  row += "<td class='center border'>" + v1 + "</td>";
  row += "<td class='center border'>" + v2 + "</td>";
  row += "</tr>" + NL;
  return row;
}

static 
String configPageInputRow3Cols(const char* desc, 
    const String& v1, const String& v2, const String& v3) {
  String row("");
  row += "<tr>";
  row += "<td class='right border grey'>" + String(desc) + "</td>";
  row += "<td class='center border'>" + v1 + "</td>";
  row += "<td class='center border'>" + v2 + "</td>";
  row += "<td class='center border'>" + v3 + "</td>";
  row += "</tr>" + NL;
  return row;
}
static
String configPageInputText(const char* id, const char* text, int size=0) 
{
  String input = NL;
  input += "<input ";
  input += "type='text' ";
  input += "id='"          + String(id)   + "' ";
  input += "name='"        + String(id)   + "' ";
  input += "placeholder='" + String(text) + "' ";
  if (size) {
    input += "maxlength="    + String(size) + " ";
    input += "size="         + String(size) + " ";
  }
  input += ">";
  return input;
}

static
String configPageInputCheckbox(const char* id, bool checked=false) 
{
  // have to put in a hidden field because of the idiocy of the checkbxx
  // behavior, namely, only changes from unchecked to checked is sent down
  // // everytig else is not
  String input = NL;
  input += "<input ";
  input += "type='hidden' ";
  input += "id='"          + String(id)   + "' ";
  input += "name='"        + String(id)   + "' ";
  input += "value='false'";
  input += ">";
  input += "<input ";
  input += "type='checkbox' ";
  input += "id='"          + String(id)   + "' ";
  input += "name='"        + String(id)   + "' ";
  input += "value='true'";
  if (checked)
    input += "checked";
  input += ">";
  return input;
}

static 
String configPageInputNumber(const char* id, int value, int isize=3, int imin=0, int imax=-99999) 
{
  String input = NL;
  input += "<input ";
  input += "type='number' ";
  input += "id='"          + String(id)    + "' ";
  input += "name='"        + String(id)    + "' ";
  input += "placeholder='" + String(value) + "' ";
//  input += "maxlength='"   + String(isize) + "' "; // not supported for number
  input += "size='"        + String(isize) + "' ";
  input += "min='"         + String(imin) + "' ";
  if (imax > imin)
    input += "max='"       + String(imax) + "' ";
  input += ">";
  return input;
}

static 
String configPageInputCombobox(const char* id, const char* choices[], int ichoice, int n) {
  String input = NL;
  if (ichoice <  0) ichoice = 0;
  if (ichoice >= n) ichoice = 0;
//  input += "<div style='height:100%'>";
  input += "<select name='" + String(id) + "'>";
  for(int i=0;i<n;i++) {
    input += "<option value='" + String(i) + "'";
    if (i==ichoice)
      input += " selected";
    input += ">" + String(choices[i]) + "</option>";
  }
//  input += "</div>";
  return input;
}

static 
String configPageInputRadio(
  const char* id, const char* choices[], const int* values, int choice, int n) {
  String input = "";
  for(int i=0;i<n;i++) {
    input += "<input type='radio' name='" + String(id) + "'";
    input += " value='" + String(values[i]) + "'";
    if (i==choice)
      input += " checked";
    input += ">" + String(choices[i]) + "  ";
  }
  return input;
}


void handleConfigClock(void) {
  DateTime dt = rtClock->now();
  String now = dt.timestamp();
  now.remove(now.length()-3); // get rid of the seconds

/*
  <head>
    <title>My Web Page</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  </head>
*/
  String field;
  String TABLE = "<table width='95%' align='center'>" + NL;
  String page = R"(<!DOCTYPE html>)";
  page += FPSTR(STYLE_HEAD);
  page += R"(
<body>
  <div style='text-align:center; min-width:260px'>
  <h3 style='text-align:center; font-weight:bold'>Countdown Clock</h3><hr><br>

  <form action="/get" method ="GET">

  <table width='95%' align='center'>
  <caption>Start/Final Message</caption>
  <col width='50%'><col width='50%'>)";

  page += R"(<tr><th>Message</th><th>Text</th></tr>)";
  field = inputFieldText("msgStart", config->_msgStart, 13);
  page += addInputRow("Start (0-12)", field);
  field = inputFieldText("msgEnd", config->_msgFinal, 13);
  page += addInputRow("Final (0-12)", field);
  page += R"(</table><br>)";

  page += R"(<table width='95%' align='center'>
  <caption>Countdown Mode</caption>
  <col width='50%'><col width='50%'>
  <tr><th>Setting</th><th>Value</th></tr>)";
  field = inputFieldComboBox("downFormat",
    formatNamesCountDown,config->getFormat(MODE_COUNTDOWN),N_FORMAT_COUNTDOWN);
  page += addInputRow("Clock style", field);

  String future( (rtClock->now() + TimeSpan(365*24*60*60)).timestamp());
  future.remove(future.length()-3); // get rid of the seconds
  field = inputFieldDateTime("downTime",future);
  page += addInputRow("Future time", field);
  page += R"(</table><br>)";

  page += R"(
    <p>
    <label for="input1">Input 1:</label>
    <input type="text" id="input1" name="input1">
    <label for="countdown-time">Countdown to: </label>
    <input type="datetime-local" id="countdown-time" name="countdown-time" value=)";

page += "\"" + now + "\" ";
page += R"(min="2000-06-07T00:00" max="2035-06-14T00:00"/><br><p>)";

page += R"(<button type='submit' name='btn' value='save' >Save</button>
    </p>
  </form>
</body> 
<script>
  var datetime = new Date();
  var isoString = datetime.toISOString();
  document.getElementById("input1").value = isoString.substr(0,isoString.length-5)
</script>
</html>)";

// page += R"(<input type="submit" value="Submit">
  PL("about to send from cnfig page");
  PL(page);
  PL(page.length());
  //server->send(200, "text/html", "Hello World");
  server->send(200, "text/html", page);
  return;


#ifdef YURIJ
<html><head>
  <meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'>
  <title>Countdown Setup</title>
  <style type='text/css'>
    table       {text-align:center; border-collapse:collapse; width=90%; margin=0px auto;}
    th,td       {padding-left: 5px; padding-right: 5px; border:1px solid blue}
    td.noborder {border: 0px;}
    td.bold     {font-weight:bold;}
    td.left     {text-align:left;}
    td.center   {text-align:center;}
    td.right    {text-align:right;}
    td.grey     {background-color:#f2f2f2;}
    caption     {font-weight:bold;}
    button      {border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:40%;}
    input[type='text'], input[type='number'] {font-size:100%; border:2px solid red}
  </style>
</head>

 String page = R"(
<!doctype html>
  <html lang='en'>)";
  page += FPSTR(STYLE_HEAD);
  page += R"(<body>
  <div style='text-align:center; min-width:260px'
  <h3 style='text-align:center; font-weight:bold'>Countdown Clock<br>)";
  page += now;
  page += R"(<hr></h3>)";
 
  /*
   * start form
   */
  //<form method='get' action='save'>)";
  page += R"(
  <form method='get' action='save'>)";
    
  /*
   * clock table
   */
  page += R"(
    <label for="meeting-time">Countdown to: </label>
    <input type="datetime-local" id="meeting-time" name="meeting-time" value=)";
  page += "\"" + now + "\" ";
  page += R"(min="2000-06-07T00:00" max="2035-06-14T00:00"/><br>)";
  page += R"(
    <button type='submit' name='btn' value='save' >Save</button>)";
//    <input type='submit' name='btn' value='save' >Save</button>)";

  /*
   * end form
   */
  page += R"(
  </form>)";
 
  /*
   * the end
   */
  page += R"(
  </div>
</body>
</html>)";
#endif
#ifdef YURIJ
  page += TABLE;
  page += "<caption>Clock Setup</caption>" + NL;
  page += "<col width='50%'><col width='50%'>" + NL;
  page += "<tr><th>Time</th><th>Value</th></tr>" + NL;
  field = configPageInputNumber("dd", day, 4, 0, 9999); 
  page += configPageInputRow("Days (0-9999)", field);
  field = configPageInputNumber("hh", hour, 4, 0, 23); 
  page += configPageInputRow("Hours (0-23)", field);
  field = configPageInputNumber("mm", minute, 4, 0, 59); 
  page += configPageInputRow("Minutes (0-59)", field);
  field = configPageInputNumber("ss", second, 4, 0, 59); 
  page += configPageInputRow("Seconds (0-59)", field);

  const int values[] = {0,1};
  const char* directions[] = {"CountDown","Clock"};
  const int   direction = config->getMode();
  field  = configPageInputRadio("dir", directions, values, direction , 2); 
  page  += configPageInputRow("Display Mode", field);
    
/*
    countdown display modes
    "0 dd D | hh:mm |  ss u",
    "1 dd D | hh:mm |    ss",
    "2 dd D | hh  H | mm:ss",
    "3 dd D | hh  H |  mm n",
    "4   dd | hh:mm |  ss u",
    "5   dd | hh:mm |    ss",
    "6   dd |    hh | mm:ss",
    "7   dd |    hh |    mm"
*/


  const char  *modes[] = {
   "dd D | hh:mm |  ss.u",
   "dd D | hh:mm |    ss",
   "dd D | hh  H | mm:ss",
   "dd D | hh  H |  mm N",
   "  dd | hh:mm |  ss.u",
   "  dd | hh:mm |    ss",
   "  dd |    hh | mm:ss",
   "  dd |    hh |    mm"
  };

  int imode = 0;
  field = configPageInputCombobox("mode", modes, config->getFormat(), sizeof(modes)/sizeof(modes[0]));
  page  += configPageInputRow("Display Mode", field);

  page += "</table><br>" + NL;

  /*
   * msg table
   */
  page += TABLE;
  page += "<caption>Start/Stop Messages</caption>" + NL;
  page += "<col width='50%'><col width='50%'>" + NL;
  page += "<tr><th>Message</th><th>Text</th></tr>" + NL;
  field = configPageInputText("msg0", config->_msgStart.c_str(), 13); 
  page += configPageInputRow("Start (0-12)", field);
  field = configPageInputText("msg1", config->_msgFinal.c_str(), 13); 
  page += configPageInputRow("End (0-12)", field);
  page += "</table><br>" + NL;


  /*
   * buttons
   */
  page += "<br>";
  page += "<button type='submit' name='btn' value='save' >Save</button>"  + NL;
  page += "<button type='submit' name='btn' value='test' >Test</button>"  + NL;
  page += "<br><br><br>";
  page += "<p><p>";

 /*
   * access point table
   */
  page += TABLE;
  page += "<caption>Access Point Wifi Settings</caption>" + NL;
  field = configPageInputText("apn", config->_apSSID.c_str(), 16); 
  page += configPageInputRow("AP Name", field);
  field = configPageInputText("app", config->_apPassword.c_str(), 16); 
  page += configPageInputRow("AP Password", field);
  page += "</table><br>" + NL;

#endif
  /*
   * external links
   */
  page += "<a href='/view' align=center><b>View Config File</b></a>" + NL;
  page += "<p>";
  page += "<a href='/delete' align=center><b>Delete Config File</b></a>" + NL;
  page += "<p>";
  /*
   * end form
   */
  page += "</form>" + NL;
  
  // server->send(200, "text/html", page.c_str());
}

void  handleConfigView(void) {
  String filename(config->getFilename());
  if (SPIFFS.exists(filename)) {
    String  context = "text/json";
    File file = SPIFFS.open(filename, "r");
    size_t sent = server->streamFile(file, "text/json");
    file.close();
  } else {
    String page("<!DOCTYPE htm>\n");
    page += "<html lang='en'><body>\n";
    page += "<h2>File '" + filename + "' Not Found</h2>";
    page += "</body></html>\n";
    server->send(200, "text/html", page);
  }
  return;
}

void  handleConfigReboot(void) {
  String page = "";
  page += "<!doctype html>"    + NL;;
  page +=   "<html lang='en'>" + NL;;
  page += FPSTR(STYLE_HEAD);
  page +=     "<body>"  + NL;  
  page +=     "<div style='text-align:center; min-width:260px;'>";
  page += "<h2 style='text-align:center; font-weight:bold'>Rebooting in 2 second</h2>";
  page += "<p><p>";
  page += "<form method='get' action='/'><button type='submit'>Home</button></form>";
  page += "</div></body></html>";
  server->send(200, "text/html", page); 
  extern  void  reboot(void);
  reboot();
  return;
}

void  handleConfigDelete(void) {
  const String&  path = config->getFilename();
  String page = "";
  page += "<!doctype html>"    + NL;;
  page +=   "<html lang='en'>" + NL;;
  page += FPSTR(STYLE_HEAD);
  page +=     "<body>"  + NL;  
  page +=     "<div style='text-align:center; min-width:260px;'>";
  
  String msg = "file '" + path + "'";
  if (SPIFFS.exists(path)) {
    SPIFFS.remove(path);
    msg += " removed";
  } else {
    msg += " not found!";
  }
  page += "<h2 style='text-align:center; font-weight:bold'>" + msg + "</h2>";
  page += "<p><p>";
  page += "<form method='get' action='/'><button type='submit'>Home</button></form>";
  page += "</div></body></html>";
  server->send(200, "text/html", page); 
  return;
}


void handleConfigSave(void) {
  String page = "";
  page += "<!doctype html>"    + NL;;
  page +=   "<html lang='en'>" + NL;;
  page +=     "<head>" + NL;
  page +=       "<meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'>" + NL;
  page +=       "<title>Countdown Setup</title>" + NL;
  page +=       "<style type='text/css'>" + NL;
  page +=         STYLE_BUTTON + NL;
  page +=       "</style>" + NL;
  page +=     "</head>" + NL;
  page +=     "<body>"  + NL;  
  page +=     "<div style='text-align:left; min-width:260px;'>";

  page += "<h4>" + NL;
  for(int i=0; i<server->args();i++) 
    page += server->argName(i) + "='" + server->arg(i) + "'<br>" + NL;
  page += "</h4>" + NL;
  page += "</div>";

  //page += "<div style='text-align:center; min-width:260px;'><br>";
  page += "<div style='text-align:center'><br>";
//  page += "<form style='display:inline-block' action='/' method='get'>";
  page += "<form action='/' method='get'>";
  page += "  <button>Home</button>";
  page += "<p>";
  page += "</form>";
//  page += "<form style='display:inline-block' action='/reboot' method='get'>";
  page += "<form action='/reboot' method='get'>";
  page += "  <button>Reboot</button>";
  page +=  "</form>\n";
  page +=  "</div></body></html>";
  Serial.println(page);
  server->send(200, "text/html", page);
      
      return;


  bool      changed = false;
  bool      changedTime = false;
  bool      changedBrightness = false;

  DateTime dt = rtClock->now();
  uint8_t day = dt.day();
  uint8_t hour = dt.hour();
  uint8_t minute = dt.minute();
  uint8_t second = dt.second();

  for(int i=0; i<server->args();i++) {
    PV(server->argName(i)); SPACE; PVL(server->arg(i));
    if (server->arg(i).length()) {
      changed = true;

      if (server->argName(i) == "dd") {
        changedTime = true;
        day = server->arg(i).toInt();
      }

      else if (server->argName(i) == "hh") {
        changedTime = true;
        hour = server->arg(i).toInt();
      }

      else if (server->argName(i) == "mm") {
        changedTime = true;
        minute = server->arg(i).toInt();
      }

      else if (server->argName(i) == "ss") {
        changedTime = true;
        second = server->arg(i).toInt();
      }

      else if (server->argName(i) == "mode") {
        config->setMode( server->arg(i).toInt() );
      }

      else if (server->argName(i) == "msg0") {
        config->_msgStart = server->arg(i);
      }

      else if (server->argName(i) == "msg1") {
        config->_msgFinal = server->arg(i);
      }

      else if (server->argName(i) == "brt") {
        config->setBrightness(server->arg(i).toInt());
        changedBrightness = true;
      }

      else if (server->argName(i) == "dir") {
        config->setMode(server->arg(i).toInt());
      }

      // wifi ap settings
      else if (server->argName(i) == "apn") {
        config->_apSSID = server->arg(i);
      }

      else if (server->argName(i) == "app")  {
        config->_apPassword = server->arg(i);
      } 
    }

    if (changedBrightness) {
      display->refresh();
      Serial.println("forcing new brightness");
    }

    if (changed) {
      config->saveFile();
    }
  
    if (server->arg("btn").equals("test")) {
      extern bool EVENT_DEMO_MODE;
      EVENT_DEMO_MODE = true;
      Serial.printf("handleConfiSave| trying test mode\n");
    }
  }
}

String getDateTimePage(void ) {
char  page[] = R"(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<meta name="viewport" content="width=device-width, initial scale=1.0">
<title>Document</title>
<h1> Time is <span id="time"> </span></h1>
<!--
    <script src="index.js">   </script>
-->
</head>
<body>
</body>
<script>
var datetime = new Date();
console.log(datetime);
document.getElementById("time").textContent = datetime; 
</script>
</html>

)";
  return page;
}

void handleSyncTime() {
  DateTime dt = rtClock->now();
  String now = dt.timestamp();
  now.remove(now.length()-3); // get rid of the seconds

  String page = R"(<!DOCTYPE html>)";
  page += FPSTR(STYLE_HEAD);
  page += R"(
<body>
  <p>Temperature is 32.26 ºC</p>
  <p>Temperature is 32.26 &deg;C</p>
  <p>Temperature is 32.26 &#x000b0;C</p>
  <p>Temperature is 32.26 &#176;C</p> 
  <p>Client Time = <span id="isodt"> </span></p>

  <form action="/get" method ="GET">
    <p>
    <label for="input1">Input 1:</label>
    <input type="text" id="input1" name="input1">
    <label for="countdown-time">Countdown to: </label>
    <input type="datetime-local" id="countdown-time" name="countdown-time" value=)";
page += "\"" + now + "\" ";
page += R"(min="2000-06-07T00:00" max="2035-06-14T00:00"/><br>)";
page += R"(<p>)";
page += R"(<button type='submit' name='btn' value='save' >Save</button>
    </p>
  </form>
</body> 
<script>
  var datetime = new Date();
  document.getElementById("isodt").textContent = datetime;
  var isoString = datetime.toISOString();
  document.getElementById("input1").value = isoString.substr(0,isoString.length-5)
</script>
</html>)";

// page += R"(<input type="submit" value="Submit">
  PL("about to send from cnfig page");
  PL(page);
  PL(page.length());
  //server->send(200, "text/html", "Hello World");
  server->send(200, "text/html", page);
}

void handleConfigGet() {
  PL("handleConfigGet")
  String page="<h1>";
  page += rtClock->now().timestamp() + "<br>" + NL;
  
  for(int i=0; i<server->args();i++) 
    page += server->argName(i) + "=|" + server->arg(i) + "| <br>" + NL;

  page += "</h1>";
  P(page);
  server->send(200, "text/html", page);
  
  PL("responded")
}
