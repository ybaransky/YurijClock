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
static  const String     SPACE(" ");
static  const String     QUOTE("'");
static  const String     TABLE = "<table width='95%' align='center'>";

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
static const String quote(const char*   text) { return QUOTE + String(text) + QUOTE; }
static const String quote(const int        i) { return QUOTE + String(i) + QUOTE; }

static String inputFieldText(const char* id, const String& text, int size=0) 
{
  String input = NL;
  String sid(id);
  String ssize(size);
  input += "<input ";
  input += "type='text' ";
  input += "id="          + quote(sid)  + SPACE;
  input += "name="        + quote(sid)  + SPACE;
  input += "placeholder=" + quote(text) + SPACE;
  if (size) {
    input += "maxlength="    + ssize + SPACE;
    input += "size="         + ssize + SPACE;
  }
  input += ">";
  return input;
}

static String inputFieldDateTime(const char* id, const String& text)
{
  String input = NL;
  String sid(id);
  input += "<input ";
  input += "type='datetime-local' ";
  input += "id="    + quote(sid)  + SPACE;
  input += "name="  + quote(sid)  + SPACE;
  input += "value=" + quote(text) + SPACE;
  input += "min='2000-06-07T00:00' max='2035-06-14T00:00'";
  input += ">";
  return input;
}

static String inputFieldComboBox(const char* id, const char* choices[], int ichoice, int n) {
  String input = NL;
  if (ichoice <  0) ichoice = 0;
  if (ichoice >= n) ichoice = 0;
//  input += "<div style='height:100%'>";
  input += "<select name=" + quote(id) + ">" + NL;
  for(int i=0;i<n;i++) {
    input += "<option value=" + quote(i); 
    if (i==ichoice) input += " selected";
    input += ">" + String(choices[i]) + "</option>" + NL;
  }
//  input += "</div>";
  return input;
}

static String inputFieldRadio(
  const char* id, const char* choices[], const int* values, int choice, int n) {
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

void handleConfigClock(void) {
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
  <h3 style='text-align:center; font-weight:bold'>Countdown Clock</h3><hr><br>
  <form action="/get" method ="GET" onsubmit='setTimeout(function() {window.location.reload();},10)'>
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
  field = inputFieldText("msgStart", config->_msgStart, 13);
  page += addInputRow("Start (0-12)", field);
  field = inputFieldText("msgEnd", config->_msgFinal, 13);
  page += addInputRow("Final (0-12)", field);
  page += R"(</table><br>

  )";

/*
 *   MODE CHOICE (Radio Buttons)
 */

  page += TABLE;
  page += R"(
  <caption>Clock Mode Choice</caption>
  <tr><td>)";
  const int values[] = {MODE_COUNTDOWN, MODE_COUNTUP, MODE_CLOCK};
  page += inputFieldRadio("mode", modeNames, values, config->getMode(), 3); 
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
  field = inputFieldComboBox("countdownFormat",
    formatNamesCountDown,config->getFormat(MODE_COUNTDOWN),N_FORMAT_COUNTDOWN);
  page += addInputRow("Format", field);
  time = (rtClock->now() + TimeSpan(180*24*60*60)).timestamp();
  time.remove(time.length()-3); // get rid of the seconds
  field = inputFieldDateTime("endTime",time);
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
  field = inputFieldComboBox("countupFormat",
    formatNamesCountUp,config->getFormat(MODE_COUNTUP),N_FORMAT_COUNTUP);
  page += addInputRow("Format", field);
  time = rtClock->now().timestamp();
  time.remove(time.length()-3); // get rid of the seconds
  field = inputFieldDateTime("startTime",time);
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
  field = inputFieldComboBox("clockFormat",
    formatNamesClock,config->getFormat(MODE_CLOCK),N_FORMAT_CLOCK);
  page += addInputRow("Format", field);
  page += R"(</table><br>

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
  field = inputFieldText("ssid", config->_apSSID, 12);
  page += addInputRow("Hotspot Name", field);
  field = inputFieldText("pswd", config->_apPassword, 10);
  page += addInputRow("Hotspot Password", field);
  page += R"(</table><br>
  )";

  /*
   * external links
   */

page += R"(
  <a href='/sync' align=center><b>Sync Clock</b></a><p>
  <a href='/view' align=center><b>View Config File</b></a><p>
  <a href='/delete' align=center><b>Delete Config File</b></a>
 
  </form>
</body> 
<script>
  var datetime = new Date();
  var isoString = datetime.toISOString();
  document.getElementById("input1").value = isoString.substr(0,isoString.length-5)
</script>
</html>)";

  PL("about to send from cnfig page");
//  PL(page);
  PL(page.length());
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
  String filename(config->getFilename());
  PVL(filename);
  if (FILESYSTEM.exists(filename)) {
    String  context = "text/json";
    File file = FILESYSTEM.open(filename, "r");
    size_t sent = server->streamFile(file, "text/json");
    file.close();
  } else {
    String msg = "File " + quote(filename) + " not found";
    server->send(200, "text/html", getFileMsg(msg));
  }
  return;
}
void  handleConfigDelete(void) {
  const String&  filename = config->getFilename();
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
  page += "<form method='get' action='/'><button type='submit'>Home</button></form>";
  page += "</div></body></html>";
  server->send(200, "text/html", page); 
  extern  void  reboot(void);
  reboot();
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

#ifdef YURIJ
  bool      changed = false;
  bool      changedTime = false;
  bool      changedBrightness = false;

  DateTime dt = rtClock->now();

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
#endif
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
  <form action="/get" method ="GET" onsubmit='setTimeout(function() {window.location.reload();},10)'>
  <label for="syncTime">Sync This</label><input type="text" id="syncTime" name="syncTime">
  <br>
  <br>
  <button type='submit' name='btn' value='Sync'>Sync</button>
  </form>
  )";


#ifdef YURIJ
  page += R"(
  <p>Clock Time = <span id="isodt"> </span></p>
  <form action="/get" method ="GET">
    <p>
    <label for="input1">Input 1:</label>
    <input type="text" id="input1" name="input1">
    <label for="countdown-time">Countdown to: </label>
    <input type="datetime-local" id="countdown-time" name="countdown-time" value= )";
  page += quote(now);
  page += R"(min="2000-06-07T00:00" max="2035-06-14T00:00"/><br>
    <p> <button type='submit' name='btn' value='save' >Save</button> </p>
 )";
 #endif
 
  page += R"(
</body> 
<script>
  var datetime = new Date();
  var isotime = datetime.toISOString();
  time = isotime.substr(0,isotime.length-5);
  document.getElementById("cellTime").textContent = time;
  document.getElementById("syncTime").value = time;
</script>
</html>)";
  P("about to send from sync page "); PVL(page.length());
  server->send(200, "text/html", page);
}

void handleConfigGet() {
  P("handleConfigGet at");PL() 
  String page="<h1>";
  page += rtClock->now().timestamp() + "<br>" + NL;
  
  for(int i=0; i<server->args();i++) {
    page += server->argName(i) + "=|" + server->arg(i) + "| <br>" + NL;
    P(i); PV(server->argName(i)); SP; PVL(server->arg(i));
  }
  page += "</h1>";
  server->send(200, "text/html", page);
  
  PL("responded")
}

#ifdef YURIJ
  page += R"(
    <p>
    <label for="input1">Input 1:</label>
    <input type="text" id="input1" name="input1">
    <label for="countdown-time">Countdown to: </label>
    <input type="datetime-local" id="countdown-time" name="countdown-time" value=)";
page += "\"" + now + "\" ";
page += R"(min="2000-06-07T00:00" max="2035-06-14T00:00"/><br><p>)";
#endif

