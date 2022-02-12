#include <Arduino.h>
#include <ShiftRegister74HC595.h>
#include <ESPRotary.h>
#include <Button2.h>

#include <ESP8266WebServer.h>
#include <WiFiManager.h>


/*************************************************
 * Public Constants
 *************************************************/

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978




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

int position = 0;

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

void handleNotFound();

void setup() {
    Serial.begin(115200);

    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);

    if(!wifiManager.autoConnect()) {
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

    // r.setChangedHandler(rotate);
    // b.setClickHandler(click);
    // b.setLongClickHandler(resetPosition);

    visualisePosition(5);
}

void loop() {
    r.loop();
    b.loop();
    server.handleClient();
}

void configModeCallback (WiFiManager *myWiFiManager) {
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

void handlePosition() {
    digitalWrite(led, 1);
    server.send(200, "text/plain", String(getPosition(r)));
    digitalWrite(led, 0);
}

void handlePositionUp() {
    digitalWrite(led, 1);
    r.resetPosition((getPosition(r) + 1) * 5);
    server.send(200, "text/plain", String(getPosition(r)));
    digitalWrite(led, 0);
}

void handlePositionDown() {
    digitalWrite(led, 1);
    r.resetPosition((getPosition(r) - 1) * 5);
    server.send(200, "text/plain", String(getPosition(r)));

    digitalWrite(led, 0);
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

void visualisePosition(int position) {
    Serial.println("visualisePosition");
    Serial.println(position);
    sendValueToLights(dataArray[position]);
}

void sendValueToLights(uint8_t value) {
    Serial.println("sendValueToLights");
    Serial.println(value);

    // uint8_t pinValues[] = { value };
    // registry.setAll(pinValues);


    customShiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, value);

    digitalWrite(LATCH_PIN, HIGH);
    digitalWrite(LATCH_PIN, LOW);
}

void customShiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
    byte i = 8;
    do{
        digitalWrite(clockPin, LOW);
        digitalWrite(dataPin, LOW);
        if(val & 0x80) digitalWrite(dataPin, HIGH);
        digitalWrite(clockPin, HIGH);
        val <<= 1;
    }while(--i);
    return;
}

/**
 * @param ESPRotary r
 */
void rotate(ESPRotary &r) {
    visualisePosition(getPosition(r));
}

void click(Button2 &btn) {
    registry.setAllHigh();
    buzz();
}

void resetPosition(Button2 &btn) {
    r.resetPosition();
    registry.setAllLow();
}

int getPosition(ESPRotary &r) {
    return round(r.getPosition() / 5);
}

void buzz() {
    tone(BUZZER_PIN, NOTE_F6);
    delay(250);
    noTone(BUZZER_PIN);
}
