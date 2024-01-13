#include <Arduino.h>
#include <chords.h>
#include <buzzer.h>
#include <ShiftRegister74HC595.h>
#include <ESP8266WebServer.h>

#include <ESPRotary.h>
#include <Button2.h>
#include <WiFiManager.h>

#define ROTARY_PIN1 D1
#define ROTARY_PIN2 D2
#define BUTTON_PIN  D3

#define CLICKS_PER_STEP 1   // this number depends on your rotary encoder, 18 clicks for 360 degrees
#define MIN_POS   0
#define MAX_POS   40

//74HC595 Pins
#define DATA_PIN D5
#define CLOCK_PIN D8
#define LATCH_PIN D7

#define BUZZER_PIN D6

ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, MIN_POS, MAX_POS);
Button2 b = Button2(BUTTON_PIN);

const int led = 13;

ESP8266WebServer server(80);
ShiftRegister74HC595<1> registry(DATA_PIN, CLOCK_PIN, LATCH_PIN);
Buzzer buzzer = {BUZZER_PIN};

uint8_t dataArray[9] = {
    B00000000,
    B00000001,
    B00000011,
    B00000111,
    B00001111,
    B00011111,
    B00111111,
    B01111111,
    B11111111
};


void rotate(ESPRotary &r);
void doubleClick(Button2 &btn);
void click(Button2 &btn);
void resetPosition(Button2 &btn);

int getPosition(ESPRotary &r);

void buzz();

void visualisePosition(int position);
void configModeCallback(WiFiManager *myWiFiManager);
void sendValueToLights(uint8_t value);

void handleBuzz();
void handleNotFound();
void handlePosition();
void handlePositionUp();
void handlePositionDown();

void setup() {  
  Serial.begin(9600);

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect()) {
    Serial.println("Failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  server.on("/position", handlePosition);
  server.on("/up", handlePositionUp);
  server.on("/down", handlePositionDown);
  server.on("/buzz", handleBuzz);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  //set pins to output so you can control the shift register
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);

  //set pins to output so you can control buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  buzzer.begin();

  r.setChangedHandler(rotate);
  b.setClickHandler(click);
  b.setDoubleClickHandler(doubleClick);
  b.setLongClickHandler(resetPosition);

  visualisePosition(0);
}

void loop() {
  r.loop();
  b.loop();
  server.handleClient();
  buzzer.refresh();
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

int getPosition(ESPRotary &r) {
  return round(r.getPosition() / 5);
}

void visualisePosition(int position) {
  sendValueToLights(dataArray[position]);
}

void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void handleBuzz() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "Buzz 8p");
  buzz();
  digitalWrite(led, 0);
}

void rotate(ESPRotary &r) {
  visualisePosition(getPosition(r));
}

void doubleClick(Button2 &btn) {
  buzz();
}

void click(Button2 &btn) {
  
  
}

void buzz() {
  buzzer.queueBeep(200, 1000);  
  buzzer.queueBeep(200, 1000);  
  buzzer.queueBeep(200, 1000);    
  buzzer.queueBeep(500, 3000);    
}

void resetPosition(Button2 &btn) {
  r.resetPosition();
  registry.setAllLow();
}

void handlePosition() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", String(getPosition(r)));
  digitalWrite(led, 0);
}

void sendValueToLights(uint8_t value) {
  uint8_t pinValues[] = { value };
  registry.setAll(pinValues);

  digitalWrite(LATCH_PIN, HIGH);
  digitalWrite(LATCH_PIN, LOW);
}

void handlePositionUp() {
  digitalWrite(led, 1);
  r.resetPosition((getPosition(r) + 1) * 5);
  visualisePosition(getPosition(r));
  server.send(200, "text/plain", String(getPosition(r)));
  digitalWrite(led, 0);
}

void handlePositionDown() {
  digitalWrite(led, 1);
  r.resetPosition((getPosition(r) - 1) * 5);
  visualisePosition(getPosition(r));
  server.send(200, "text/plain", String(getPosition(r)));

  digitalWrite(led, 0);
}
