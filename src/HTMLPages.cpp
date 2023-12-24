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
static  const String     idMode("mode");
static  const String     idCDFormat("cdFormat");
static  const String     idCUFormat("cuFormat");
static  const String     idCLFormat("clFormat");
static  const String     idTimeEnd("timeEnd");
static  const String     idTimeStart("timeStart");
static  const String     idSSID("ssid");
static  const String     idPassword("password");
static  const String     idSyncTime("syncTime");


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

static const String quote(const String& text) { return QUOTE + text + QUOTE; }
static const String quote(const int        i) { return QUOTE + String(i) + QUOTE; }

static void pageInfo(const char* msg, const String& page)  {
  P(msg); SPACE; P(page.length()); PL(" bytes sent");
}

static String inputFieldText(const String& id, const String& text, int size=0) 
{
  String input = NL;
  String ssize(size);
  input += "<input ";
  input += "type='text' ";
  input += "id="          + quote(id)  + BLANK;
  input += "name="        + quote(id)  + BLANK;
  input += "placeholder=" + quote(text) + BLANK;
  if (size) {
    input += "maxlength="    + ssize + BLANK;
    input += "size="         + ssize + BLANK;
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

void handleConfigSetup(void) {
  DateTime dt = rtClock->now();
  String now = dt.timestamp();
  now.remove(now.length()-3); // get rid of the seconds

  String time;
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

/*
 *   START / STOP  MESSAGE
 */

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

/*
 *   MODE CHOICE (Radio Buttons)
 */

  page += TABLE;
  page += R"(
  <caption>Clock Mode</caption>
  <tr><td>)";
  const int values[] = {MODE_COUNTDOWN, MODE_COUNTUP, MODE_CLOCK};
  page += inputFieldRadio(idMode, modeNames, values, config->getMode(), 3); 
  page += R"(</td></tr>
  </table><br>

  )";

/*
 *   Countdown mode
 */

  page += TABLE;
  page += R"(
  <caption>Countdown Mode Setup</caption>
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

/*
 *   Countup mode
 */

  page += TABLE;
  page += R"(
  <caption>Countup Mode Setup</caption>
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

/*
 *   Clock mode
 */

  page += TABLE;
  page += R"(
  <caption>Clock Mode Setup</caption>
  <col width='50%'><col width='50%'>
  )";
  //<tr><th>Setting</th><th>Value</th></tr>
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
/*
 *   Form Buttons
 */

page += R"(
     <p>
     <button type='submit' name='btn' value='save' >Save</button>
     <button type='submit' name='btn' value='test' >Test</button>
    </p>
    <br> <br>
    )";

/*
 *   Network Settings
 */

  page += TABLE;
  page += R"(<caption>Access Point Wifi Settings</caption>
  <col width='50%'><col width='50%'>
  )";
  //<tr><th>Message</th><th>Text</th></tr>
  field = inputFieldText(idSSID, config->_apSSID, 12);
  page += addInputRow("Hotspot Name", field);
  field = inputFieldText(idPassword, config->_apPassword, 10);
  page += addInputRow("Hotspot Password", field);
  page += R"(</table><br>
  )";

  /*
   * external links
   */

page += R"(
  <a href='/sync'   align=center><b>Sync Clock</b></a><p>
  <a href='/view'   align=center><b>View Config File</b></a><p>
  <a href='/delete' align=center><b>Delete Config File</b></a>
  <a href='/reboot' align=center><b>Reboot Clock</b></a>
 
  </form>
</body> 
</html>)";

  pageInfo("handleConfigClock", page);
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
  int changed = 0;
  bool guiUpdate = false;
  bool reboot = false;
  int value;

  if (server->arg("btn").equals("test")) {
    extern bool EVENT_DEMO_START;
    EVENT_DEMO_START = true;
    server->send(200, "text/html", "entered test mode");
    return;
  }

  String page="<h1>";
  for(int i=0; i<server->args();i++) {
    page += server->argName(i) + "=|" + server->arg(i) + "| <br>" + NL;
    P(i); P(") argName=|");P(server->argName(i)); P("| arg=|"); P(server->arg(i));PL("|");
    if (server->arg(i).length()) {

      if (server->argName(i) == idMode) {
        value = server->arg(i).toInt();
        if (config->getMode() != value) {
          config->setMode(value);
          guiUpdate = true;
          changed++;
        }
      }
      else if (server->argName(i) == idMsgStart) {
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
      else if (server->argName(i) == idSSID) {
        config->_apSSID = server->arg(i);
        changed++;
        reboot = true;
      }
      else if (server->argName(i) == idPassword)  {
        config->_apPassword = server->arg(i);
        changed++;
        reboot = true;
      } 
    }
  }
  //PVL(changed);
 
  if (guiUpdate) {
    display->refresh(); 
  }

  if (changed>0) {
    config->saveFile();
  }

  if (reboot) {
    extern void reboot(void);
    reboot();
  }

  page += "</h1>";
  pageInfo("handleConfigSave",page);
  server->send(200, "text/html", "done");
}

void handleSyncTime() {
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
  <form action="/syncget" method ="GET" onsubmit='setTimeout(function() {window.location.reload();},10)'>
  )";
  page += "<label for=" + quote(idSyncTime) + ">Sync to this </label>";
  page += "<input type='text' id=" + quote(idSyncTime) + " name=" + quote(idSyncTime) + ">";
  page += R"(
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
  pageInfo("handleSyncTime",page);
  server->send(200, "text/html", page);
  //<a href="/"><button>wReturn</button></a>
}


void handleSyncTimeGet() {
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
  pageInfo("handleSyncTimeGet",page);
  server->send(200, "text/html", page);
}