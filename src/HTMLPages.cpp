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
static  const String     idBrightness("brightness");

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
    button:hover {background-color:aquamarine};
    input[type='text'], input[type='number'] {font-size:100%; border:2px solid red}
  </style>
</head>
)";

//.flex       {display: flex; gap:0.3rem;}
//    button      {border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:40%;}
static const String quote(const String& text) { return QUOTE + text + QUOTE; }
static const String quote(const int        i) { return QUOTE + String(i) + QUOTE; }

static void pageInfo(const char* msg, const String& page, ulong start)  {
  ulong ms = millis() - start;
  P(msg); SPACE; P(page.length()); P(" bytes sent in "); P(ms); PL("ms");
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
  String field = "<button type='submit' name='btnHome' value=";
  field += quote(id);
  if (active) 
    field += String(" style='background-color:aquamarine;'");
  field += " >" + String(text) + "</button><br><br>" + NL;
  return field;
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

static int brightnessToComboBox(void) {
  // {high,med,low}  
  switch(config->getBrightness()) {
    case 1:
    case 2: return 2; 
    case 3:
    case 4:
    case 5: return 1; 
    case 6:
    case 7: 
    default: return 0;
  }
}
static int comboBoxToBrightness(int value) {
  switch(value) {
    case 2: return 1;
    case 1: return 4;
    case 0:
    default: return 7;
  }
}

static void handleClockSave(void) {
  const char* fcn="handleClockSave:";
  int changed = 0;
  bool guiUpdate = false;  
  String row;
    
  for(int i=0; i<server->args();i++) {
    row = server->argName(i) + "=|" + server->arg(i) + "| <br>";
    P(i); P(") "); PL(row);

    if (server->arg(i).length()) {
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
      } else if (server->argName(i) == idBrightness) {
        int brightness = comboBoxToBrightness(server->arg(i).toInt());
        if ((config->getBrightness()) != brightness) {
          config->setBrightness(brightness);
          changed++;
          guiUpdate = true;
        }
      }
    }
  }
 
  if (guiUpdate) {
    display->refresh(fcn); 
  }

  if (changed>0) {
    PL("should save config");
  //   config->saveFile();
  }
  P(fcn); P(" changed=");PL(changed);
}

static void handleSyncSave(void) {
  const char* fcn = "handleSyncSave:";
  PL(fcn);
  String row;
  for(int i=0; i<server->args();i++) {
    row = server->argName(i) + "=|" + server->arg(i) + "|";
    P(i); P(") "); PL(row);
    if (server->arg(i).length()) {
      if (server->argName(i) == idSyncTime) {
        rtClock->adjust(DateTime(server->arg(i).c_str()));
      }
    }
  }
}

static void handleWifiSave(void) {
  const char *fcn = "handleWifiSave:";
  PL(fcn);

  bool reboot = false;
  if (server->arg("btnWifi").equals("reboot")) {
    reboot = true;
  } else {
    String row;
    for(int i=0; i<server->args();i++) {
      row = server->argName(i) + "=|" + server->arg(i) + "| <br>";
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
    }
  }

  if (reboot) {
    PL("should  save config and reboot")
    extern void reboot(void);
  } 
}

static void handleHomeSave(void) {
  const char* fcn="handleHomeSave";
  PL(fcn);
  for(int i=0; i<server->args();i++) {
    PL(String(i) + ") " + server->argName(i) + " len=" + String(server->arg(i).length()) + " arg==|" + server->arg(i) + "|");
    if (server->arg("btnHome").equals(idCDBtn)) {
      config->setMode(MODE_COUNTDOWN,fcn);
    }
    else if (server->arg("btnHome").equals(idCUBtn)) {
      config->setMode(MODE_COUNTUP,fcn);
    }
    else if (server->arg("btnHome").equals(idCLBtn)) {
      config->setMode(MODE_CLOCK,fcn);
    }
    else if (server->arg("btnHome").equals(idDemoBtn)) {
      extern bool EVENT_DEMO_START;
      EVENT_DEMO_START = true;
    }
  }
}

void handleHome(void) {
  const char* fcn = "handleHome";
  PL(fcn);
  if (server->arg("btnHome").equals(idCDBtn) || 
      server->arg("btnHome").equals(idCUBtn) ||
      server->arg("btnHome").equals(idCLBtn) || 
      server->arg("btnHome").equals(idDemoBtn)) {
    handleHomeSave();
  }

  ulong start = millis();
  String page = R"(<!DOCTYPE html>)";
  page += FPSTR(STYLE_HEAD);
  page += R"(
<body>
  <div style='text-align:center; min-width:260px'>
  <h3 style='text-align:center; font-weight:bold'>)";
  page += config->getSSID();
  page += R"(</h3><hr>
  <br>
  <form method ="GET">
  )";
  int mode = config->getMode();
  page += addRootButton(idCDBtn,  "Count Down", mode==MODE_COUNTDOWN);
  page += addRootButton(idCUBtn,  "Count Up",   mode==MODE_COUNTUP);
  page += addRootButton(idCLBtn,  "Clock",      mode==MODE_CLOCK);
  page += addRootButton(idDemoBtn,"Demo",       mode==MODE_DEMO);
  page += R"(
  </form>
  <br><br><br><br><br><br>
  <a href='/clock'  align=center><b>Clock Setup</b></a><p>
  <a href='/wifi'   align=center><b>Wifi Setup</b></a><p>
  <a href='/view'   align=center><b>View Config File</b></a><p>
  <a href='/delete' align=center><b>Delete Config File</b></a><p>
  )";
  pageInfo(fcn, page, start);
  server->send(200, "text/html", page);
}

void handleWifi(void) {
  const char* fcn="handleWifi";
  PL(fcn);
  if (server->arg("btnWiFi").equals("savewifi") ||
      server->arg("btnWifi").equals("reboot")) {
    handleWifiSave();
  }

  ulong start = millis();

  String field;
  String page = R"(<!DOCTYPE html>)";
  page += FPSTR(STYLE_HEAD);
  page += R"(
<body>
  <div style='text-align:center; min-width:260px'>
  <h3 style='text-align:center; font-weight:bold'>Wifi Setup</h3><hr><br>
  <form method='GET'>
  )";
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

  <p>
  <button type='submit' name='btnWifi' value='savewifi'>Save</button>
  <button type='submit' name='btnWifi' value='reboot'>Reboot</button>
  <p><p>
  <button type='submit' name='btnWifi' value='home' formation='/'>Home</button>
  </form> 
  </body>
</html>
 )";
  pageInfo(fcn,page,start);
  server->send(200, "text/html", page);
  return;
}

void handleClock(void) {
  const char* fcn="handleClock";
  PL(fcn);
  if (server->arg("btnClock").equals("save")) {
    handleClockSave();
  } 

  ulong start = millis();
  String field;

  String page = R"(<!DOCTYPE html>)";
  page += FPSTR(STYLE_HEAD);
  page += R"(
<body>
  <div style='text-align:center; min-width:260px'>
  <h3 style='text-align:center; font-weight:bold'>)";
  page += config->getSSID();
  page += R"( Setup</h3><hr>
  <form method='GET'>
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
 * Brightness  
 ********************************************************************/
  page += TABLE;
  page += R"(
  <caption>Brightness</caption>
  <col width='50%'><col width='50%'>
  )";
  const char*  brightnessNames[] = {"High","Medium","Low"};
  field = inputFieldComboBox(idBrightness, brightnessNames ,brightnessToComboBox(),3);
  page += addInputRow("Brightness Value", field);  
  page += R"(</td></tr> 
    </table><br>

 )";
/*********************************************************************
 * Form Buttons
 ********************************************************************/
page += R"(
  <p>
  <button type='submit' name='btnClock' value='save'>Save</button> 
  <button type='submit' name='btnClock' value='sync' formaction='/sync'>Sync Time</button>
  <p>
  <button type='submit' name='btnClock' value='home' formaction='/'>Home</button>
  </form> 
  </body> 
</html>)";

  pageInfo(fcn, page, start);
  server->send(200, "text/html", page);
  return;
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
    const char* fcn = "handleConvigView:";
    ulong start = millis();
    String msg = "File " + quote(filename) + " not found";
    String page = getFileMsg(msg);
    pageInfo(fcn,page,start);
    server->send(200, "text/html", page);
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

void handleSync() {
  const char* fcn = "handleSync:";
  PL(fcn);
  if (server->arg("btnSync").equals("sync")) {
    handleSyncSave();
  }

  ulong start = millis();
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
  page += R"( Time</td><td class='left border grey'>)"; 
  page += now; 
  page += R"(</td></tr> 
  <tr><td class='right border grey'>)"; 
  page += "Cell Phone";
  page += R"( Time</td><td class='left border grey'> <label id='cellTime'> </label></td></tr>
  </table>
  <br>
  <form method ="GET">
  )";
  page += "<h2 <label for=" + quote(idSyncTime) + ">Sync to this </label>";
  page += "<br><input type='text' style='font-size:large; border-width=5px; border-color:red;' maxlength='20' size='20' id=" + quote(idSyncTime) + " name=" + quote(idSyncTime) + "></h2>";
  page += R"(
  <p>
  <br>
  <br>
  <button type='submit' name='btnSync' value='sync'>Sync It</button>
  <button type='submit' name='btnSync' value='refresh'>Refresh</button>
  <p>
  <p>
  <button type='submit' name='btnSync' value='home' formaction='/'>Home</button>
  </form>
  <script>
    var dt = new Date();
    dt.setMinutes(dt.getMinutes() - dt.getTimezoneOffset());
    var isotime = dt.toISOString();
    isotime = isotime.substr(0,isotime.length-5);
    document.getElementById("cellTime").textContent = isotime;
    document.getElementById("syncTime").value = isotime;
  </script>
  </body> 
</html>)";
  pageInfo(fcn,page,start);
  server->send(200, "text/html", page);
}

