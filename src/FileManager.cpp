#ifdef YURIJ

#include <Arduino.h>
#include <FS.h>
#include "wifi_manager.h"
#include "history_config.h"
#include "history_server.h"
#include "html_pages.h"

static  int sPrevYear = 988;

static const char HTTP_HEAD_0[] PROGMEM  = \
"<head>\n"\
"  <meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=yes'/>\n"\
"  <meta charset='UTF-8'>\n"\
"  <meta name='author' content='Yurij Baransky'>\n"\
"  <title>{title}</title>\n"\
"  <style>\n"\
"   .calign {text-align:center;}\n"\
"   .ralign {text-align:right;}\n"\
"   .lalign {text-align:left;}\n"\
"   .bold   {font-weight:bold;}\n"\
"   .input_box {width:80px; text-align:center; border:2px solid red; border-raduis:4px; line-height:1.6rem; font-size:1.2rem;}\n"\
"   .button {margin:auto; display:block;\n"\
"            border:0; border-radius:0.3rem; width:25%;\n"\
"            background-color:#1fa3ec; color:#fff;\n"\
"            line-height:2.4rem; font-size:1.2rem;}\n"\
"    table  {margin:auto; border-collapse:collapse;}\n"\
"    table, th, td {border:2px solid blue;}\n"\
"    th, td {padding-left:4px; padding-right:4px; text-align: center;}\n"\
"    tr:nth-child(even){background-color: #f2f2f2}\n"\
"  </style>\n"\
"</head>\n";

static String getContentType(String filename) {
  if(filename.endsWith(".htm"))       return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".json")) return "text/json";
  else if(filename.endsWith(".css"))  return "text/css";
  else if(filename.endsWith(".png"))  return "image/png";
  else if(filename.endsWith(".gif"))  return "image/gif";
  else if(filename.endsWith(".jpg"))  return "image/jpeg";
  else if(filename.endsWith(".ico"))  return "image/x-icon";
  else if(filename.endsWith(".xml"))  return "text/xml";
  else if(filename.endsWith(".pdf"))  return "application/x-pdf";
  else if(filename.endsWith(".zip"))  return "application/x-zip";
  else if(filename.endsWith(".gz"))   return "application/x-gzip";
  else if(filename.endsWith(".js"))   return "application/javascript";
return "text/plain";
}

static String getPageTitle(const char* title) {
  return "<h3 class='calign bold'>" + String(title) + "<hr></h3>\n";
}

static String getHeadTitle(const char* title) {
  String headTitle = FPSTR(HTTP_HEAD_0);
  headTitle.replace("{title}",title);
  headTitle.replace("{style}","");
  return headTitle;
}

static String getPage() {
  String head = getHeadTitle("Yurij's Page");
  String page("<!DOCTYPE htm>\n");
  page += "<html lang='en'>\n";
  page += head;
  page += "<body>\n";
  page += getPageTitle("Sensor Page");
  page += "This should work\n";
  page += "<p>\n";
  page += "<table width='75%'>\n";
  page += "<tr><th>Property</th><th>Value</th></tr>\n";
  page += "<tr><td class='ralign'>Network</td>   <td class='lalign'>Some name</td></tr>\n";
  page += "<tr><td class='ralign'>IP Addr</td>   <td class='lalign'>192.168.1.10</td></tr>\n";
  page += "<tr><td class='ralign'>more stuff</td><td class='lalign'>100/sec</td></tr>\n";
  page += "</table>\n";
  page += "<p>\n";
  page += "<div><form action='/home' method='get'><button>Home</button></form></div>\n";
  page += "</body>\n";
  page += "</html>\n";
  return page;
}

void handleDirectory(void) {
  String page("<!DOCTYPE htm>\n");
  page += "<html lang='en'>\n";
  page += getHeadTitle("ESP8266 Directory");
  page += "<body>\n";
  String  directory = String(gConfig->_ap_name) + " Directory";
  page += getPageTitle(directory.c_str());
  page += "<p>\n";

  Dir dir = SPIFFS.openDir("/");
  page += "<table width='100%'>";
  page += "<tr><th>File</th><th></th><th>Size</th><th></th></tr>\n";
  while (dir.next()) {    
    String path = dir.fileName();
    size_t size = dir.fileSize();
    page += "<tr>";
    page += "<td>" + path + "</td>";
    page += "<td><a href=/file?path=" + path + "&action=view>View</a></td>";
    page += "<td>" + String(size) + "</td>";
    page += "<td><a href=/file?path=" + path + "&action=delete>Delete</a></td>";
    page += "</tr>\n";
 //   page += "<a href=/file?path=" + path + ">" + path + "</a> " + String(size) + "<br>\n";
  }
  page += "</table>\n";

  page += "<svg version='1.1' baseProfile='full' width='300' height='200' xmlns='http://www.w3.org/2000/svg'>\n";
  page += "<rect width='100%' height='100%' fill='red' />\n";
  page += "<circle cx='150' cy='100' r='80' fill='green' />\n";
  page += "<text x='150' y='125' font-size='60' text-anchor='middle' fill='white'>SVG</text></svg>\n";
  page += "<div><form action='/home' method='get'><button>Home</button></form></div>\n";
  page += "</body></html>\n";
  server->send(200, "text/html", page);
}

static 
String htmlInputNumber(const char* id, int value=1000, int imin=0, int imax=-99999) 
{
  String input("\n");
  input += "<input type='number' ";
  input += "class='input_box' ";
  input += "id='"          + String(id)    + "' ";
  input += "name='"        + String(id)    + "' ";
  input += "placeholder='" + String(value) + "' ";
  input += "min='"         + String(imin) + "' ";
  if (imax > imin)
    input += "max='"       + String(imax) + "' ";
  input += ">\n";
  return input;
}

static 
String htmlInputSubmit(const char* id)
{
  String input("\n");
  input += "<input type='submit' ";
  input += "class='button' ";
  input += "value='" + String(id) + "' ";
  input += ">\n";
  return input;
}

void handleGetYear(void) {
  String page("<!DOCTYPE htm>\n");
  page += "<html lang='en'>\n";
  page += getHeadTitle("History");
  page += "<body>\n";
  String  directory = String(gConfig->_ap_name);
  page += getPageTitle(directory.c_str());
  page += "<p>\n";

  page += "<form method='get' action='history'>\n";
  page += "<h3>";
  page += "What happend in ";
  page += htmlInputNumber("year",sPrevYear,1,2018);
  page += " A.D. ?</h3>";
  page += "<p><br>";
  page += htmlInputSubmit("Submit");
  page += "</form>";

  page += "</body></html>\n";
//  Serial.println(page);
  Serial.printf("page has %d bytes\n",page.length());
  server->send(200, "text/html", page);
}

void handleTable(void) {
  String page("<!DOCTYPE htm>\n");
  page += "<html lang='en'>\n";
  page += getHeadTitle("History Directory");
  page += "<body>\n";
  String  directory = String(gConfig->_ap_name) + " Directory";
  page += getPageTitle(directory.c_str());
  page += "<p>\n";

  page += "<table width='100%'>";
  for(int row=30;row<40;row++) {
    page += "<tr>";
    for(int i=0;i<5;i++) {
      String year(row*10+i);
      page += "<td><a href=/history?year=" + year + ">" + year + "</a></td>";
    }
    page += "</tr>\n";
  }
  page += "</table>\n";
  page += "<div><form action='/' method='get'><button>Home</button></form></div>\n";
  page += "</body></html>\n";
//  Serial.println(page);
  Serial.printf("page has %d bytes\n",page.length());
  server->send(200, "text/html", page);
}

void  handleShowHistory(void) 
{
  int year=-1;
  for ( uint8_t i = 0; i < server->args(); i++ ) 
  {
    if (server->argName(i).equals("year")) 
    {
      year = server->arg(i).toInt();
      if (year < 1)     year = 1;
      if (year > 2018)  year = 2018;
      sPrevYear = year;
    }
  }

  const HistoryYear& history = gHistory->find_history(year);

  String page("<!DOCTYPE htm>\n");
  page += "<html lang='en'>\n";
  page += getHeadTitle("History Directory");
  page += "<body>\n";
  String  title = "In the year " + String(year) + " A.D.";
  page += getPageTitle(title.c_str());
  page += "<p>\n";

  if (!history.num_events()) 
  {
    page += "nothing found !";
  } 
  else 
  {
    if (history.year() != year) 
    {
      page += "<div style='calign'>Nothing happened, so moving to " 
        + String(history.year()) + " A.D.</div><p>";
    }
    history.display_html(page);
  }

  page += "<p>\n";
  page += "<div><form action='/' method='get'><button class='button'>Home</button></form></div>\n";
  page += "</body></html>\n";
  server->send(200, "text/html", page);
}

void  handleFile(void) {
  String  path("");
  String  action("");
  for ( uint8_t i = 0; i < server->args(); i++ ) {
    if (server->argName(i).equals("path")) 
      path = server->arg(i);
    else if (server->argName(i).equals("action"))
      action = server->arg(i);
  }

  if (SPIFFS.exists(path)) {
    if (action.equals("view")) {
      String  context = getContentType(path);
      File file = SPIFFS.open(path,"r");
      size_t sent = server->streamFile(file, context);
      file.close();
      return;
    } else if (action.equals("delete")) {
      SPIFFS.remove(path);
      server->send(200, "text/html", "<h1>File:" + path + " removed</h1>");
      return;
    }
  }

  gWifiManager.handleNotFound();
  return;
}

#endif