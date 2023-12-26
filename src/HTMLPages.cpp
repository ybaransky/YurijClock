#include <Arduino.h>
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include "Constants.h"
#include "Config.h"
#include "Display.h"
#include "RTClock.h"

extern Display    *display;
extern RTClock    *rtClock;
extern Config     *config;
extern WebServer  *server;

static  const String     NL("\r\n");
static  const String     EMPTY("");
static  const String     BLANK(" ");
static  const String     QUOTE("'");
static  const String     TABLE = "<table width='95%' align='center'>";

static  const String     idMsgStart("msgStart");
static  const String     idMsgEnd("msgEnd");
static  const String     idCDFormat("cdFormat");
static  const String     idCUFormat("cuFormat");
static  const String     idCLFormat("clFormat");
static  const String     idTimeEnd("timeEnd");
static  const String     idTimeStart("timeStart");
static  const String     idSSID("ssid");
static  const String     idPassword("password");
static  const String     idSyncTime("syncTime");
static  const String     idCDBtn("cdBtn");
static  const String     idCUBtn("cuBtn");
static  const String     idCLBtn("clBtn");
static  const String     idDemoBtn("demoBtn");


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
    button      {border-radius:0.3rem;line-height:2.4rem;font-size:1.2rem;width:40%;}
    input[type='text'], input[type='number'] {font-size:100%; border:2px solid red}
  </style>
</head>
)";

//    button      {border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:40%;}
static const String quote(const String& text) { return QUOTE + text + QUOTE; }
static const String quote(const int        i) { return QUOTE + String(i) + QUOTE; }

static void pageInfo(const char* msg, const String& page)  {
  P(msg); SPACE; P(page.length()); PL(" bytes sent");
}

static String inputFieldText(const String& id, const String& text, int size=0) 
{
  String input = NL;
  input += "<input ";
  input += "type='text' ";
  input += "id="          + quote(id)  + BLANK;
  input += "name="        + quote(id)  + BLANK;
  input += "placeholder=" + quote(text) + BLANK;
  if (size) {
    input += "maxlength="    + quote(size) + BLANK;
    input += "size="         + quote(size) + BLANK;
  }
  input += ">";
  return input;
}

static String inputFieldDateTime(const String& id, const String& text)
{
  String input = NL;
  input += "<input ";
  input += "type='datetime-local' ";
  input += "id="    + quote(id)  + BLANK;
  input += "name="  + quote(id)  + BLANK;
  input += "value=" + quote(text) + BLANK;
  input += "min='2000-06-07T00:00' max='2035-06-14T00:00'";
  input += ">";
  return input;
}

static String inputFieldComboBox(const String& id, const char* choices[], int ichoice, int n) {
  String input = NL;
  if (ichoice <  0) ichoice = 0;
  if (ichoice >= n) ichoice = 0;
//  input += "<div style='height:100%'>";
  input += "<select name=" + quote(id) + ">" + NL;
  for(int i=0;i<n;i++) {
    input += "<option value=" + quote(i); 
    if (i==ichoice) input += " selected='selected'";
    input += ">" + String(choices[i]) + "</option>" + NL;
  }
  return input;
}

#ifdef YURIJ
static String inputFieldRadio(const String& id, const char* choices[], const int* values, int choice, int n) {
  String input = "";
  for(int i=0;i<n;i++) {
    input += "<input type='radio' name=" + quote(id) + " value=" + quote(values[i]);
    if (i==choice)
      input += " checked";
    input += ">" + String(choices[i]) + "  ";
  }
  return input;
}
#endif

static String addInputRow(const char* desc, const String& value) {
  String row("");
  row += R"(<tr><td class='right border grey'>)";
  row += String(desc);
  row +=R"(</td><td class='left border'>)";
  row += value;
  row += R"(</td></tr>
  )";
  return row;
}

static String addRootButton(const String& id, const char* text,  bool active) {
  //<button type='submit' name='btn' value='cntdn' style='background-color:aquamarine;'>Count Down</button>
  String field = "<button type='submit' name='btn' value=";
  field += quote(id);
  if (active) 
    field += String(" style='background-color:aquamarine;'");
  field += " >" + String(text) + "</button><br><br>" + NL;
  return field;
}

void handleWifiSetup(void) {
  const char* fcn="handleNetworkSetup";

  String field;
  String page = R"(<!DOCTYPE html>)";
  page += FPSTR(STYLE_HEAD);
  page += R"(
<body>
  <div style='text-align:center; min-width:260px'>
  <h3 style='text-align:center; font-weight:bold'>Wifi Setup</h3><hr><br>
  <form action='/wifi' method='GET'>
  )";
  //<form action="/save" method ="GET" onsubmit='setTimeout(function() {window.location.reload();},10)'>
/*********************************************************************
 *   Network 
 ********************************************************************/
  page += TABLE;
  page += R"(
  <caption>Access point Setup</caption>
  <col width='50%'><col width='50%'>
  )";
  field = inputFieldText(idSSID, config->_apSSID, 12);
  page += addInputRow("Hotspot Name", field);
  field = inputFieldText(idPassword, config->_apPassword, 10);
  page += addInputRow("Hotspot Password", field);
  page += R"(</table><br>

  <p><button type='submit' formtarget='_self' name='btn' value='savewifi' >Save</button></p> 
  </form> 
  <a href='/reboot' align=center><b>Reboot Clock</b></a>
 )";
  PL(page);
  pageInfo(fcn,page);
  PL("hey!!!!");
  if (server->arg("btn").equals("savewifi")) {
    PL("this came from the wifei sabe button")
  } else {
    PL("this came from the elsewhere")
  }
  server->send(200, "text/html", page);
}

void handleClockSetup(void) {
  const char* fcn="handleClockSetup";
  //String time;
  String field;

  String page = R"(<!DOCTYPE html>)";
  page += FPSTR(STYLE_HEAD);
  page += R"(
<body>
  <div style='text-align:center; min-width:260px'>
  <h3 style='text-align:center; font-weight:bold'>)";
  page += config->getSSID();
  page += R"( Setup</h3><hr><br>
  <form action="/save" method ="GET" onsubmit='setTimeout(function() {window.location.reload();},10)'>
  )";
/*********************************************************************
 *   Countdown mode
 ********************************************************************/
  page += TABLE;
  page += R"(
  <caption>Countdown Setup</caption>
  <col width='50%'><col width='50%'>
  )";
  //<tr><th>Setting</th><th>Value</th></tr>
  field = inputFieldComboBox(idCDFormat,
    formatNamesCountDown,config->getFormat(MODE_COUNTDOWN),N_FORMAT_COUNTDOWN);
  page += addInputRow("Format", field);
  field = inputFieldDateTime(idTimeEnd,config->getTimeEnd());
  page += addInputRow("End time", field);
  page += R"(</table><br>

  )";
/*********************************************************************
 *   Countup mode
 ********************************************************************/
  page += TABLE;
  page += R"(
  <caption>Countup Setup</caption>
  <col width='50%'><col width='50%'>
  )";
  //<tr><th>Setting</th><th>Value</th></tr>
  field = inputFieldComboBox(idCUFormat,
    formatNamesCountUp,config->getFormat(MODE_COUNTUP),N_FORMAT_COUNTUP);
  page += addInputRow("Format", field);
  field = inputFieldDateTime(idTimeStart,config->getTimeStart());
  page += addInputRow("Start time", field);
  page += R"(</table><br>

  )";
/*********************************************************************
 *   Clock mode
 ********************************************************************/
  page += TABLE;
  page += R"(
  <caption>Clock Setup</caption>
  <col width='50%'><col width='50%'>
  )";
  field = inputFieldComboBox(idCLFormat,
    formatNamesClock,config->getFormat(MODE_CLOCK),N_FORMAT_CLOCK);
  page += addInputRow("Format", field);  
  page += R"(<tr><td class='right border grey'>)";
  page += config->_apSSID; 
  page += R"( Time</td><td class='left border'>)"; 
  page += rtClock->now().timestamp(); 
  page += R"(</td></tr> 
    </table><br>

 )";
/*********************************************************************
 * Start/Stop Messages
 ********************************************************************/
  page += TABLE;
  page += R"(
  <caption>Start/Final Message</caption>
  <col width='50%'><col width='50%'>
  )";
  //<tr><th>Message</th><th>Text</th></tr>
  field = inputFieldText(idMsgStart, config->getMsgStart(), 13);
  page += addInputRow("Start (0-12)", field);
  field = inputFieldText(idMsgEnd, config->getMsgEnd(), 13);
  page += addInputRow("Final (0-12)", field);
  page += R"(</table><br>

  )";
/*********************************************************************
 * Form Buttons
 ********************************************************************/
page += R"(
  <p>
  <button type='submit' name='btn' value='saveclock' >Save</button> 
  </form> 
  <form method='GET' action='/sync'>
  <button type='submit' name='btn' value='sync' >Sync Time</button> 
  </form> 

  </body> 
</html>)";

  pageInfo(fcn, page);
//  PL(page);
  server->send(200, "text/html", page);
  return;
}

static String getFileMsg(const String& msg) {
  String page = "<!doctype html><html lang='en'>";
  page += FPSTR(STYLE_HEAD);
  page += R"(
    <body>
    <div style='text-align:center; min-width:260px;'>
    <h3 style='text-align:center; font-weight:bold'>
    )";
  page += msg;
  page += R"(</h3>
    <p><p>
    <form method='get' action='/'><button type='submit'>Home</button></form>
    </div></body></html>
  )";
  return page;
}

void  handleConfigView(void) {
  String filename(config->getFileName());
  PVL(filename);
  if (FILESYSTEM.exists(filename)) {
    String  context = "text/json";
    File file = FILESYSTEM.open(filename, "r");
    size_t sent = server->streamFile(file, "text/json");
    P("filesize="); PL(sent);
    file.close();
  } else {
    String msg = "File " + quote(filename) + " not found";
    server->send(200, "text/html", getFileMsg(msg));
  }
  return;
}

void  handleConfigDelete(void) {
  const String&  filename = config->getFileName();
  String msg = "File " + quote(filename);
  if (FILESYSTEM.exists(filename)) {
    FILESYSTEM.remove(filename);
    msg += " removed";
  } else {
    msg += " not found!";
  }
  server->send(200, "text/html", getFileMsg(msg)); 
  return;
}

void  handleReboot(void) {
  String page = "";
  page += "<!doctype html>"    + NL;;
  page +=   "<html lang='en'>" + NL;;
  page += FPSTR(STYLE_HEAD);
  page +=     "<body>"  + NL;  
  page +=     "<div style='text-align:center; min-width:260px;'>";
  page += "<h2 style='text-align:center; font-weight:bold'>Rebooting in 2 second</h2>";
  page += "<p><p>";
//  page += "<form method='get' action='/'><button type='submit'>Home</button></form>";
  page += "</div></body></html>";
  server->send(200, "text/html", page); 
  extern  void  reboot(void);
  delay(2000);
  reboot();
  return;
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

static bool changedFormat(int mode,const String& id, int value, int& changed) {
  bool rc = config->getMode() != mode;
  if (id == idCDFormat) {
    P("mode="); P(mode); 
    P(" format "); P(config->getFormat(mode)); P(" --> "); P(value);
    PL("");
  }
  if (config->getFormat(mode) != value) {
      config->setFormat(value, mode);
      changed++;
  }
  return rc;
}

void handleConfigSave() {
  const char* fcn = "handleConfigSave";
  String page="<h1>";
  String row;

  if (server->arg("btn").equals("savewifi")) {
    bool reboot = false;
    // this is from the WiFiSetup page
    for(int i=0; i<server->args();i++) {
      row = server->argName(i) + "=|" + server->arg(i) + "| <br>";
      page += row + NL;
      P(i); P(") "); PL(row);
      if (server->argName(i) == idSSID) {
        if (config->getSSID() != server->arg(i)) {
          config->setSSID(server->arg(i));
          reboot = true;
        }
      } else if (server->argName(i) == idPassword)  {
        if (config->getPassword() != server->arg(i)) {
          config->setPassword(server->arg(i));
          reboot = true;
        }
      }
      handleWifiSetup();
      if (reboot) {
        PL("should  save config and reboot")
        extern void reboot(void);
      } 
      return;
    }
  } else if (server->arg("btn").equals("saveclock")) {
    int value;
    int changed = 0;
    bool guiUpdate = false;  
    // this is from the ClockSetup page
    for(int i=0; i<server->args();i++) {
      row = server->argName(i) + "=|" + server->arg(i) + "| <br>";
      page += row + NL;
      P(i); P(") "); PL(row);

      //if (server->arg(i).length()) {
      if (false) {
        if (server->argName(i) == idMsgStart) {
          if (config->getMsgStart() != server->arg(i)) {
            config->setMsgStart(server->arg(i));
            changed++;
          }
        }
        else if (server->argName(i) == idMsgEnd) {
          if (config->getMsgEnd() != server->arg(i)) {
            config->setMsgEnd(server->arg(i));
            changed++;
          }
        }
        else if (server->argName(i) == idCDFormat) {
          if (changedFormat(MODE_COUNTDOWN,idCDFormat,server->arg(i).toInt(),changed)) {
            guiUpdate = true;
          }
        }
        else if (server->argName(i) == idCUFormat) {
          if (changedFormat(MODE_COUNTUP,idCUFormat,server->arg(i).toInt(),changed)) {
            guiUpdate = true;
          }
        }
        else if (server->argName(i) == idCLFormat) {
          if (changedFormat(MODE_CLOCK,idCLFormat,server->arg(i).toInt(),changed)) {
            guiUpdate = true;
          }
        }
        else if (server->argName(i) == idTimeStart) {
          if (config->getTimeStart() != server->arg(i)) {
            config->setTimeStart(server->arg(i));
            changed++;
          }
        }
        else if (server->argName(i) == idTimeEnd) {
          if (config->getTimeEnd() != server->arg(i)) {
            config->setTimeEnd(server->arg(i));
            changed++;
          }
        }
      } 
    }
    PVL(changed);
 
    if (guiUpdate) {
      display->refresh(fcn); 
    }

    if (changed>0) {
      PL("should save config");
    //   config->saveFile();
    }
    handleClockSetup();
  }
}

void handleSyncSetup() {
  const char* fcn = "handleSyncSetup";
  DateTime dt = rtClock->now();
  String now = dt.timestamp();
//  now.remove(now.length()-3); // get rid of the seconds

  String page = R"(<!DOCTYPE html>)";
  page += FPSTR(STYLE_HEAD);

  page += R"(
<body>
  <div style='text-align:center; min-width:260px'>
  <h3 style='text-align:center; font-weight:bold'>Sync Clock to Cell Phone</h3><hr><br>
  )";
  page += TABLE;
  //<col width='50%'><col width='50%'>
  page += R"(
  <caption>Hardware Times</caption>
  <tr><td class='right border grey'>)";
  page += config->_apSSID; 
  page += R"( Time</td><td class='left border'>)"; 
  page += now; 
  page += R"(</td></tr> 
  <tr><td class='right border grey'>)"; 
  page += "Cell Phone";
  page += R"( Time</td><td class='left border'> <label id='cellTime'> </label></td></tr>
  </table>
  <br>
  <br>
  <form action="/syncsave" method ="GET" onsubmit='setTimeout(function() {window.location.reload();},10)'>
  )";
  page += "<label for=" + quote(idSyncTime) + ">Sync to this </label>";
  page += "<input type='text' id=" + quote(idSyncTime) + " name=" + quote(idSyncTime) + ">";
  page += R"(
  <p>
  <br>
  <br>
  <button type='submit' name='btn' value='Sync'>Sync</button>
  </form>
  <br> 
  <form method='get' action='/'><button type='submit'>Home</button></form>
</body> 

<script>
  var dt = new Date();
  dt.setMinutes(dt.getMinutes() - dt.getTimezoneOffset());
  var isotime = dt.toISOString();
  isotime = isotime.substr(0,isotime.length-5);
  document.getElementById("cellTime").textContent = isotime;
  document.getElementById("syncTime").value = isotime;
</script>

</html>)";
  pageInfo(fcn,page);
  server->send(200, "text/html", page);
}

void handleSyncSave() {
  const char* fcn="handleSyncSave";
  String page="<h1>";
  for(int i=0; i<server->args();i++) {
    page += server->argName(i) + "=|" + server->arg(i) + "| <br>" + NL;
    if (server->arg(i).length()) {
      //P(i); P(") argName=|");P(server->argName(i)); P("| arg=|"); P(server->arg(i));PL("|");
      if (server->argName(i) == idSyncTime) {
        rtClock->adjust(DateTime(server->arg(i).c_str()));
      }
    }
  }
  page += "</h1>";
  pageInfo(fcn,page);
  server->send(200, "text/html", page);
}



void handleRoot(void) {
  const char* fcn = "handleRoot";
  String page = R"(<!DOCTYPE html>)";
  page += FPSTR(STYLE_HEAD);
  page += R"(
<body>
  <div style='text-align:center; min-width:260px'>
  <h3 style='text-align:center; font-weight:bold'>)";
  page += config->getSSID();
  page += R"(</h3><hr>
  <br>
  <form action="/mode" method ="GET" onsubmit='setTimeout(function() {window.location.reload();},10)'>
  )";
  int mode = config->getMode();
  page += addRootButton(idCDBtn,  "Count Down", mode==MODE_COUNTDOWN);
  page += addRootButton(idCUBtn,  "Count Up",   mode==MODE_COUNTUP);
  page += addRootButton(idCLBtn,  "Clock",      mode==MODE_CLOCK);
  page += addRootButton(idDemoBtn,"Demo",       mode==MODE_DEMO);
  page += R"(
  </form>
  <br><br><br><br><br><br>
  <a href='/setup'  align=center><b>Clock Setup</b></a><p>
  <a href='/wifi'   align=center><b>Wifi Setup</b></a><p>
  <a href='/view'   align=center><b>View Config File</b></a><p>
  <a href='/delete' align=center><b>Delete Config File</b></a><p>
  )";
  #ifdef YURIJ
  <form action="/setup" method ="GET">
  <button type='submit' name='btn' value='clksetup' >Clock Setup</button><br><br>
  </form>
  <form action="/network" method ="GET">
  <button type='submit' name='btn' value='netsetup' >Wifi Setup</button><br><br>
  </form>
  #endif
  pageInfo(fcn,page);
  server->send(200, "text/html", page);
}

void handleClockMode(void) {
  const char* fcn="handleClockMode";
  String page;

  if (server->arg("btn").equals(idCDBtn))
    config->setMode(MODE_COUNTDOWN,fcn);
  else if (server->arg("btn").equals(idCUBtn))
    config->setMode(MODE_COUNTUP,fcn);
  else if (server->arg("btn").equals(idCLBtn))
    config->setMode(MODE_CLOCK,fcn);
  else if (server->arg("btn").equals(idDemoBtn)) {
    extern bool EVENT_DEMO_START;
    EVENT_DEMO_START = true;
    server->send(200, "text/html", "entered demo mode");
  }
  display->refresh(fcn);

  for(int i=0; i<server->args();i++) {
    PL(String(i) + ") " + server->argName(i) + " len=" + String(server->arg(i).length()) + " arg==|" + server->arg(i) + "|");
  }
  pageInfo(fcn,page);
  server->send(200, "text/html", page);
}