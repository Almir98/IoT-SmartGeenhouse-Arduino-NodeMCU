#include "DHT.h"
#include <Servo.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>     

// Communication between Arduino and NodeMCU
SoftwareSerial linkSerial(5, 6); // RX, TX

#define DHTPIN 2            // PINS
#define illuminancePIN A0
#define LEDPIN 13
#define servoPIN 9
#define ventilationPIN 11
#define DHTTYPE DHT11       

Servo servo_motor;         
DHT dht(DHTPIN, DHTTYPE);  //Initialization of DHT11 sensor

int optimalTemperature = 27;
int optimalHumidity = 50;
int optimalLuminosity = 50;
int pos=0;

void setup(){
  Serial.begin(115200);
  
  dht.begin();
  pinMode(LEDPIN, OUTPUT);
  servo_motor.attach(servoPIN);
  pinMode(ventilationPIN, OUTPUT);
  while (!Serial) continue;
  linkSerial.begin(4800);
}

void loop() {
  delay(15000); // Wait 10 seconds between measurements.

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float farenheit = dht.readTemperature(true);
  float heat = dht.computeHeatIndex(farenheit, humidity);
  heat = (heat - 32) * 5 / 9;
  int luminosity = analogRead(illuminancePIN);
  luminosity = map (luminosity, 0, 1023, 0, 100);

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C\t ");
  Serial.print("Luminosity: ");
  Serial.print(luminosity);
  Serial.println(" %\t ");
  Serial.print("Heat index: ");
  Serial.print(heat);
  Serial.println(" °C\t ");

  // MAIN LOGIC
  if (temperature >= optimalTemperature || humidity > optimalHumidity)
  {
    OpenWindow();
    FanON();
  }
  else if (temperature <= optimalTemperature)
  {
    CloseWindow();
    FanOFF();
  }
  if (luminosity < optimalLuminosity)
  {
    LightON();
  }
  else if (luminosity >= optimalLuminosity)
  {
    LightOFF();
  }
    // sending data ->Create the JSON document
    
    StaticJsonDocument<1024> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["heat"] = heat;
    doc["luminosity"] = luminosity;
    
    // Send the JSON document over the "link" serial port
    serializeJson(doc, linkSerial);
}

// WINDOWS functions
void OpenWindow() {
  for (pos = 0; pos <= 90; pos += 10) {
    servo_motor.write(pos);
  }
}

void CloseWindow() {
  for (pos = 90; pos >= 0; pos -= 10) {
    servo_motor.write(pos);
  }
}

// LIGHT functions
void LightON() {
  digitalWrite(LEDPIN, HIGH);
}

void LightOFF() {
  digitalWrite(LEDPIN, LOW);
}

// FAN functions
void FanON() {
  digitalWrite(ventilationPIN, HIGH);
}

void FanOFF() {
  digitalWrite(ventilationPIN, LOW);
}
