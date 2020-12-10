#include <Arduino.h>
#include <Arduino_JSON.h>
#include <ArduinoJson.h>  // radi doc-a
#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

SoftwareSerial linkSerial(D6,D5); // RX, TX

const char *ssid = "Apartmani";  //ENTER YOUR WIFI SETTINGS
const char *password = "apartmani2018";

const char *host = "s94.wrd.app.fit.ba";
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80

//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "9a8075c5fbfeeb418ed0d4c31c213ce839e9cdac";
//=======================================================================
//                    Power on setup
//=======================================================================

void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot

  linkSerial.begin(4800);
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  int temp = 10;
  
  String getData, Link;
  
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient

  if (linkSerial.available())
  {
    Serial.print("\n======= Usao u Link serial =========\n");

    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, linkSerial);

    if (err == DeserializationError::Ok)    
      {
        Serial.print("\n======= Deserijalizacija podataka  =========\n");

        int temperature = doc["temperature"].as<int>();
        int humidity = doc["humidity"].as<int>();
        int heat = doc["heat"].as<int>();
        int luminosity = doc["luminosity"].as<int>();

       Serial.print("\t\t Temperatura : ");
       Serial.println(temperature);
       Serial.print("\t\t Vlaznost zraka : ");
       Serial.println(humidity);
       Serial.print("\t\t Heat : ");
       Serial.println(heat);
       Serial.print("\t\t Osvjetljenje : ");
       Serial.println(luminosity);

        //POST Data
        Link = "/Home/Save?temperature=";
        Link+=String(temperature);
        Link+="&humidity=";
        Link+=String(humidity);
        Link+="&heat=";
        Link+=String(heat);
        Link+="&luminosity=";
        Link+=String(luminosity);
        
        Serial.print("\n========================= URL LINK =========================\n");
        Serial.println(Link);                

     }
  }
  Serial.print("\n========================= PRIPREMA SLANJA HTTPS =========================\n");

  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);
  
  Serial.print("HTTPS Connecting");
  int r=0; //retry counter
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }
  if(r==30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }
  //Link = "/Home/Save?temperature=28&humidity=34&heat=27&luminosity=79";

  Serial.print("requesting URL: ");
  Serial.println(host);
  /*
   POST /post HTTP/1.1
   Host: postman-echo.com
   Content-Type: application/x-www-form-urlencoded
   Content-Length: 13
  
   say=Hi&to=Mom
    
   */

  httpsClient.print(String("POST ") + Link + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: text/json"+ "\r\n" +
               "Content-Length: 38" + "\r\n\r\n" +
               "{\"temperature\":temperature,\"humidity\":100,\"heatIndex\":\1,\"luminosity\":1}" + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
                  
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  Serial.println("reply was:");
  Serial.println("=========== RESPONSE ==============");
  String line;
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");
    
  delay(10000);  //POST Data at every 10 seconds
}
//=======================================================================
