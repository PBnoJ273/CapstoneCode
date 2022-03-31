//LoRa Sender/Reciever Code For Water Level Testing
//By: Puneet Singh Bagga
//Capstone Project: Green Roof Sensor System
#include <heltec.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define BAND 915E6 //Setting the LoRa Band to Match US/CAN Standards -> 915MHz
//const int water_level_pin = 12; -> Originally for using a water level sensor
//#define SOUND_SPEED 0.034 //define speed of sound, needed for calculation later

//temp sensor access
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

//pins for SD Card Communication
#define SCK 17
#define MISO 19
#define MOSI 23
#define CS 5

//pins for ultrasonic sensors 
const int trigger = 12;
const int echo = 13;

long duration;
int distanceCm;
int distanceInch;
long distanceMm;

File myFile;

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  300        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

void setup() {

  pinMode(trigger, OUTPUT); //set the trigger pin as an output
  pinMode(echo, INPUT); //this is what we read in from the ultrasonic sensor
  
  // put your setup code here, to run once:
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  //Setup Display for Testing
  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Capstone Sensor Test Program");
  Heltec.display->display();
  delay(2000);
}

unsigned int counter = 0;

void loop() {
  //set the timer for the microcontroller to sleep
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  delay(100);
  //send to sleep
  esp_deep_sleep_start(); //turns off peripherals too
  // put your main code here, to run repeatedly:
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Water Level: ");
  //int level = analogRead(water_level_pin);
  //float mappedValue = mapSensor(level, 533.0, 0.0, 0.0, 4.0);
  //Grab the reading from the ultrasonic sensor
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  //set the trig pin to high to get the data 
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  //read the pulse from the echo out from the sensor, gets the travel time
  duration = pulseIn(echo, HIGH);

  //calculate the distance
  distanceCm = duration * 0.6*((temprature_sens_read() - 32) / 1.8)/2;
  distanceMm = distanceCm * 10;
  
  Heltec.display->drawString(90, 0, String(distanceMm));
  Heltec.display->drawString(0, 30, "Sending Packet: ");
  Heltec.display->drawString(90, 30, String(counter));
  Heltec.display->display();

  // send packet
  LoRa.beginPacket();
/*
 * LoRa.setTxPower(txPower,RFOUT_pin);
 * txPower -- 0 ~ 20
 * RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
 *   - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
 *   - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
*/
  LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
  //LoRa.print("Water Level: ");
  LoRa.print(distanceMm);
  LoRa.endPacket();

  if (!SD.begin(4)) {
    while (1);
  }

  myFile = SD.open("data.txt", FILE_WRITE);
  if (myFile) {
    myFile.println("Water Level: " + String(distanceMm));
    // close the file:
    myFile.close();
  }
  
  counter++;
  delay(2000);
}

float mapSensor(int value, float sensor_max, float sensor_min, float out_min, float out_max){
  return (float(value) - sensor_min) * (out_max - out_min) / (sensor_max - sensor_min) + out_min;
}
