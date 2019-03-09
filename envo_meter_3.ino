#include <FS.h>                   //SPIFFS
//#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"         //https://github.com/electronicsguy/ESP8266/blob/master/HTTPSRedirect/GoogleDocs.ino ( dit is de laatste versie de namen van de oude en nieuwe bestanden zijn gelijk)
#include <ESP8266httpUpdate.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include "dallas.h"
#include "oled_display.h"
#include "psm5003st.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <Ticker.h>  //Ticker Library voor de watchdog
volatile bool signal = false;
Ticker watchdog;

#define BAUD_RATE 115200

// Button to update the firmware
const int updateButton = 3;  // D9

//json
StaticJsonBuffer<300> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();



//Web update via  NAS
#define BOOT_AFTER_UPDATE true  // zorgt er voor dat de esp boot na remote image laden

int versie = 03;

char Google_Sheet_token[60] = "";
char Download_server [80] = "";
char App[2] = "";
String response [5] ;

//parameters op inlogscherm voor veranderen SSID, password etc.
WiFiManagerParameter custom_Google_Sheet_token("Google_Sheet", "Google_Sheet_token", Google_Sheet_token, 60);
WiFiManagerParameter custom_Download_server("Download_server", "Download_server", Download_server, 70);
WiFiManagerParameter custom_App("MM of SM", "Nummer of S", App, 2);

//flag for saving data
bool shouldSaveConfig = false;

// Push data on this interval
int dataPostDelay = 5000;  // 1 minutes = 1 * 60 * 1000

//Google server info
const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";
const int httpsPort =  443;
HTTPSRedirect*  client = nullptr;


float v [8] = {1, 2, 3, 4, 5, 6, 7, 8}; // values


long now, lastMsg , lastMsg1;

void setup() {

  Serial.begin(115200, SERIAL_8N1 );
  Serial.flush();
  pinMode(updateButton, INPUT_PULLUP);
  //  clean FS, for testing
  //SPIFFS.format();
  // watchdog om cpu te resetten als er drie minuten geen loop afgerond is
  watchdog.attach(180, check);  // loop moet elke drie minuten signal op true zetten...


  //read parameters stored in SPIFFS
  delay(5000);
  read_configuration_from_SPIFFS ();  //  Google sheet token. Download URL address

  WiFiManager wifiManager;

  initdisplay(versie, App[0]);
  show(  "connect: " + WiFi.SSID(), "probeer");
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(300);  // een timeout van 5 minuten op het configuration portal.
  //extra parameters presented in ESP8266 AP web interface
  wifiManager.addParameter(&custom_Google_Sheet_token);
  wifiManager.addParameter(&custom_Download_server);
  wifiManager.addParameter(&custom_App);

  if (!digitalRead(updateButton)) {
    if (!wifiManager.startConfigPortal("SlimmePeter", "Sagitta12345")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep s
      ESP.reset();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }


  if (shouldSaveConfig) {
    save_configuration_inti_SPIFFS ();
  }
  initdisplay(versie, App[0]);  // App[0] is de UNIT  naam
  delay(10000);

  Serial.println(" IP address: ");
  Serial.println(WiFi.localIP());


  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setContentTypeHeader("application/json");

  // connect met Google host
  Serial.print(String("Connecting to "));
  Serial.println(host);
  show("connecting to: ", host);
  connectGoogle();

  // initialiseer sensors
  // init_dallas();
  init_psm5003();

}

void loop() {

  // Update firmware ?
  if (!digitalRead(updateButton)) {
    show(  "Update software?", "druk update button meer dan 5 sec");
    if ( !digitalRead(updateButton)) {
      delay(5000) ;
    };
    if ( !digitalRead(updateButton)) {
      WebUpdate();
    };
  };
  now = millis();

  //Update  measurements
  if (now - lastMsg1 > 5000) {
    lastMsg1 = now;
    // lees de Dallas temperaturr sensors uit
    //float temp [2];
    delay(1);
    //read_dallas( temp);
    //Serial.print("temperatuur:  "); Serial.println(temp[0]);


    // lees the fijnestof sensor uit
    float result [8];
    // index  0 = pm2,5 1 =  pm1.0   2 = pm10  3 = Formaldehyde 4 = Temperatuur 5 = humidity
    if (read_psm5003(result)) {  // alleen veranderen wanner de output oke is
      for (int i = 0; i < 7; i++) {
        v[i] = result[i];
        Serial.print(i); Serial.print("   :  "); Serial.println(v[i]);
      }
    }
    initdisplay(versie, App[0] );
    delay(1000);
    //showwaardenl(String w1,String w2,String w3,String w4,String w5,String w6, String w7 , int v1, int v2, int v3, int v4, int v5, int v6, int v7);
    delay(2000);
    showcloudinfo(response );

  }

  // stuur de informatie naar de Google sheet via postData()
  if (now - lastMsg >  dataPostDelay) {
    postData();
    lastMsg = now;
  }

// laat weten aan de watchdog timer dat de loop nog leeft.
  signal = true;  // yes I'm alive
}

// Push de measured data to the  Google sheet ...................................................

void postData() {
  // reset HTTPSRedirect
  static int error_count = 0;
  static int connect_count = 0;
  const unsigned int MAX_CONNECT = 20;
  bool flag = false;
  String respons;

  //------------------message to Google sheets in the Cloud-----------
  Serial.println(flag);
  if (!flag) {
    client = new HTTPSRedirect(httpsPort);
    Serial.println( "check flag");
    flag = true;
    client->setContentTypeHeader("application/json");
  }

  if (client != nullptr) {
    Serial.println("check nulptr");
    if (!client->connected()) {
      Serial.println("verstuur");
      client->connect(host, httpsPort);
      String urlFinal = String("/macros/s/") + Google_Sheet_token + "/exec" ;
      //json
      StaticJsonBuffer<300> jsonBuffer2;
      JsonObject& root2 = jsonBuffer2.createObject();
      JsonArray& values = jsonBuffer2.createArray();
      char buffer [100];
      root2["values"] =  values;
      for (int i = 0; i < 6; i++) {
        values.add( v[i]); 
        //Serial.println(v[i]);
      };
      root2.printTo(buffer , sizeof(buffer));
      Serial.println(buffer);
      boolean verstuurd = client->POST(urlFinal, host, buffer );
      if (verstuurd == false) {
        Serial.println("geen verbinding");
        error_count ++;
        show( "Retry connection: ", String(error_count));
        delay(2000);
        if (error_count  == 4) {
          ESP.restart();
        }; // restart esp8266

      };
      respons = client->getResponseBody();
      Serial.print( "response:  ");
      Serial.println(respons);
    }
  }

  // delete HTTPSRedirect object
  delete client;
  client = nullptr;
  // ----eo - message to Google sheets in the Cloud -------------

  Serial.println("split msg");
  int index = 0;
  int previndex = 0;
  int word = 0;
  char info [30] = "" ;
  while (  respons.indexOf('\n', previndex + 1) != -1) { // zolang gevonden
    index = respons.indexOf('\n', previndex + 1);
    respons.substring(previndex, index).toCharArray(info, 29) ;
    Serial.print("index "); Serial.println(index);
    Serial.print("previndex "); Serial.println(previndex);
    Serial.println(info);
    previndex = index + 1;
    switch (word) {
      case 1:
        response[0] = info;
        break;
      case 2:
        response[1] = info;
        break;
      case 3:
        response[2] = info;
        break;
      case 4:
        response [3] = info;
        break;
      case 5:
        { response [4] = info;
          dataPostDelay = response[4].toInt() * 1000;
          Serial.print("datapost update"); Serial.println( dataPostDelay) ;
          break;
        }
    } word++;

  }

}
//Webupdate from NAS  -----------------------------------------------------------------

void WebUpdate() {

  delay(1000);
  Serial.println();
  Serial.println();
  Serial.println();


  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    show(  "setup wait", String(t));
    Serial.flush();
    delay(1000);
  }
  show(  "software update", "WAIT!!");
  delay(1000);
  ESPhttpUpdate.rebootOnUpdate(BOOT_AFTER_UPDATE);  // true
  // vereenvoudiging voor de gebruiker. Alleen de naam van het bestand.
  String basis =  "http://sagittaweb.synology.me/ESP_bin/" ;
  char tail [] = ".ino.bin";
  basis.concat(Download_server);
  basis.concat(tail);
  char server_sag [80] = " ";
  basis.toCharArray(server_sag , 80);
  Serial.println(basis);
  t_httpUpdate_return ret = ESPhttpUpdate.update(server_sag);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      show(  "http udate fout", ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}


//WiFiManager
//callback notifying us of the need to save config
void saveConfigCallback () {
  shouldSaveConfig = true;
}

bool read_configuration_from_SPIFFS () {
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(Google_Sheet_token, json["Google_Sheet_token"]);
          strcpy(Download_server, json["Download_server"]);
          strcpy(App, json["App"]);
          return true;
        } else {
          Serial.println("failed to load json config");
          return false;
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
    return false;
  }
  //end read
}

bool save_configuration_inti_SPIFFS () {
  //read updated parameters
  strcpy(Google_Sheet_token, custom_Google_Sheet_token.getValue());
  strcpy(Download_server, custom_Download_server.getValue());
  strcpy(App, custom_App.getValue());
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["Google_Sheet_token"] =  Google_Sheet_token;
  json["Download_server"] =    Download_server;
  json["App"] =    App;
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }
  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
  //end save
}
void check()
{
  Serial.println( "check & restart");
  if (signal == true) {
    Serial.println("no worries");
    signal = false;
  }
  else {
    delay(10000); Serial.println("restart");
    ESP.restart();
  }
}

boolean connectGoogle() {
  // connect to Google host
  bool flag = false;
  for (int i = 0; i < 10; i++) {
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
    delay(2000);
    show(  "connection failed: ", "opnieuw");

  }
  // delete HTTPSRedirect object
  delete client;
  client = nullptr;
  return flag;

}


