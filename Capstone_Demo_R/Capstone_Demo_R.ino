//Reciever Cod
//Set up google sheet and push data to that sheet from ESP32
#include "WiFi.h"
#include "HTTPClient.h"
#include "heltec.h" 
#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
String rssi = "RSSI --";
String packSize = "--";
String packet ;
const char* ssid = "Bagga's 3";
const char* password = "83799713";

// Gscript ID and required credentials
//String GScriptId = "AKfycby3QhKkUq0fK801xH80s8gX4GyoYOebB4d-mizHoQF402q7aMlUedRjYdg8oHRvftyD8A";    // change Gscript ID
void startWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" IP address: ");
  Serial.println(WiFi.localIP());
  
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 0, "Connected To: ");
  Heltec.display->drawString(0 , 30, ssid); 
  Heltec.display->display();
  Serial.flush(); 
}

void postData(String tag, String value){
  String url="https://script.google.com/macros/s/AKfycbxatD3SwCiFRkcFo8ZCVIWomRQA76pozyrlVYMVWDpa6N0z2Gs8euBbhYEQoXB0yWMbgw/exec?";
  Serial.print("Making a request");
  String urlFinal = url + "tag=" + tag + "&value=" + value;
  HTTPClient http;
  Serial.print(urlFinal.c_str());
  Serial.print("Making a request");
  http.begin(urlFinal); //Specify the URL and certificate
  int httpCode = http.GET();  
  http.end();
  Serial.println(": done "+httpCode);
}

void LoRaData(){
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 15 , "Received "+ packSize + " bytes");
  Heltec.display->drawString(0 , 30 , "Water Level:");
  Heltec.display->drawStringMaxWidth(0 , 40 , 128, packet);
  Heltec.display->drawString(0, 0, rssi);  
  Heltec.display->display();
}

void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  LoRaData();
}

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  Heltec.display->drawString(0, 10, "Wait for incoming data...");
  Heltec.display->display();
  delay(1000);
  startWiFi(); //start the wifi connection 
  //LoRa.onReceive(cbk);
  LoRa.receive();
}

void loop() {
  // put your main code here, to run repeatedly:
  int packetSize = LoRa.parsePacket();
  if (packetSize) { 
    cbk(packetSize);
    postData("WaterLevel", packet); 
  }
  delay(10);
}
