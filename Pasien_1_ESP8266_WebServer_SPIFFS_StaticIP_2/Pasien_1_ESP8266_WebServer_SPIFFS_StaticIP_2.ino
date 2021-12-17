// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
  #include <SPIFFS.h>
#else
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <FS.h>
#endif
#include <string.h>

// Replace with your network credentials
const char* ssid = "AA";
const char* password = "SATUSATU";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

/* Variable Global*/
int DataReceive1; //SpO2
int DataReceive2; //Heart Rate
int DataReceive3; //Pasien
float DataReceive4; //Temperature

String sendMAX30102HeartRate() {
  return String(DataReceive2);
}

String sendMAX30102SpO2() {
  return String(DataReceive1);
}

String sendMAX30102Temperature() {
  return String(DataReceive4);
}

/* Parsing the receiving data */
String getValue(String data, char separator, int index){
  int i;
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
 
  for(i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  } 
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMP"){
    return sendMAX30102Temperature();
  }
  else if(var == "HR"){
    return sendMAX30102HeartRate();
  }
  else if(var == "OXY"){
    return sendMAX30102SpO2();
  }
  return String();
}

void setup() {
  Serial.begin(115200);

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(" ");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS 1: ");
  Serial.println(WiFi.dnsIP(0));
  Serial.print("DNS 2: ");
  Serial.println(WiFi.dnsIP(1));
  
  IPAddress local_IP(192, 168, 137, 48);
  IPAddress gateway(192, 168, 137, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(local_IP, gateway, subnet);
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(" ");
  Serial.print("New IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS 1: ");
  Serial.println(WiFi.dnsIP(0));
  Serial.print("DNS 2: ");
  Serial.println(WiFi.dnsIP(1));

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/images.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/images.png", "images/png");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/apps.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/apps.js", "text/js");
  });
  server.on("/heartrate", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", sendMAX30102HeartRate().c_str());
  });
  server.on("/SpO2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", sendMAX30102SpO2().c_str());
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", sendMAX30102Temperature().c_str());
  });

  // Start server
  server.begin();
}

void loop() {
  /*Receiving Data From Client */
  String RxStr = "";
  while(Serial.available()){
    RxStr += char(Serial.read());
  }
  Serial.println(RxStr);
  
  if(RxStr != "\0"){  
    DataReceive1 = getValue(RxStr, ';', 0).toInt();
    DataReceive2 = getValue(RxStr, ';', 1).toInt();
    DataReceive3 = getValue(RxStr, ';', 2).toInt();
    DataReceive4 = getValue(RxStr, ';', 3).toFloat();

    
    Serial.print("DataReceive2[0]: ");Serial.println(DataReceive1);
    Serial.print("DataReceive2[1]: ");Serial.println(DataReceive2);
    Serial.print("DataReceive1[2]: ");Serial.println(DataReceive3);
    Serial.print("DataReceive1[3]: ");Serial.println(DataReceive4);
  }
  delay(100);
}
