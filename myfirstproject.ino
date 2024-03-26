#include <Firebase_Arduino_WiFiNINA.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define FIREBASE_HOST "myfirstproject-fa65c-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_AUTH "lQL8ehEa4s5sFAzNqaFI1VIE8OfwOm2oU9fGIoiF"
FirebaseData firebaseData;

#define WIFI_SSID "Telekom-738176"
#define WIFI_PASSWORD "drt23523b2dcddcp"
/*
#define WIFI_SSID "redmi"
#define WIFI_PASSWORD "pivojezivot"
*/

const int belt = 13;
const int red_led = 12;
const int green_led = 11;
const int blue_led = 10;

int belt_ide = 0;

const int trig1 = 9;
const int echo1 = 8;

const int trig2 = 6;
const int echo2 = 7;

const int sensor = 5;

int frequencyRED = 0;
int frequencyGREEN = 0;
int frequencyBLUE = 0;

float duration1, duration2, distance1, distance2;
String color = "";

// Define NTP client and server settings
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  Serial.begin(9600);

  Serial.print("Attempting to connect to WiFi...");
  Serial.println();
  int wifi_status = WL_IDLE_STATUS;
  while (wifi_status != WL_CONNECTED) {
    wifi_status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);
  }
  Serial.print("successfully connected to IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
  Firebase.reconnectWiFi(true);

  pinMode(belt, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(blue_led, OUTPUT);

  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

  pinMode(sensor, INPUT);

  digitalWrite(A0, HIGH);
  digitalWrite(A1, LOW);

  digitalWrite(trig1, HIGH);
  digitalWrite(trig2, HIGH);

  // Initialize NTP client
  timeClient.begin();
}

void loop() {
  digitalWrite(trig1, LOW);
  delayMicroseconds(2);
  digitalWrite(trig1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig1, LOW);

  duration1 = pulseIn(echo1, HIGH);
  distance1 = (duration1 * 0.0343) / 2;
  delay(100);
  if (distance1 < 10) {
    belt_ide = 1;
    Serial.print("distance1 = ");
    Serial.println(distance1);
  }
  if (belt_ide == 1) {
    digitalWrite(belt, HIGH);
    Serial.println("idem...");
  }

  digitalWrite(trig2, LOW);
  delayMicroseconds(2);
  digitalWrite(trig2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig2, LOW);
  duration2 = pulseIn(echo2, HIGH);
  distance2 = (duration2 * 0.0343) / 2;
  delay(100);

  Serial.println(distance1, distance2);
  if (distance2 < 10) {
    Serial.print("distance2 = ");
    Serial.println(distance2);
    delay(2000);
    belt_ide = 0;
    digitalWrite(belt, LOW);
    Serial.println("stojim...");

    //red color
    digitalWrite(A2, LOW);
    digitalWrite(A3, LOW);
    frequencyRED = pulseIn(sensor, LOW);
    frequencyRED = map(frequencyRED, 25, 72, 255, 0);
    Serial.print("R = ");
    Serial.print(frequencyRED);
    Serial.print("  ");
    delay(100);

    //green color
    digitalWrite(A2, HIGH);
    digitalWrite(A3, HIGH);
    frequencyGREEN = pulseIn(sensor, LOW);
    frequencyGREEN = map(frequencyGREEN, 30, 90, 255, 0);
    Serial.print("G = ");
    Serial.print(frequencyGREEN);
    Serial.print("  ");
    delay(100);

    //blue color
    digitalWrite(A2, LOW);
    digitalWrite(A3, HIGH);
    frequencyBLUE = pulseIn(sensor, LOW);
    frequencyBLUE = map(frequencyBLUE, 25, 70, 255, 0);
    Serial.print("B= ");
    Serial.print(frequencyBLUE);
    Serial.println("  ");
    delay(100);

    if (frequencyBLUE > frequencyRED & frequencyBLUE > frequencyGREEN) {
      //modra high
      digitalWrite(red_led, LOW);
      digitalWrite(green_led, LOW);
      digitalWrite(blue_led, HIGH);
      color = "blue";

    }
    if (frequencyBLUE < frequencyRED & frequencyRED > frequencyGREEN) {
      //red high
      digitalWrite(red_led, HIGH);
      digitalWrite(green_led, LOW);
      digitalWrite(blue_led, LOW);
      color = "red";

    }
    if (frequencyGREEN > frequencyRED & frequencyBLUE < frequencyGREEN) {
      //zelena high
      digitalWrite(red_led, LOW);
      digitalWrite(green_led, HIGH);
      digitalWrite(blue_led, LOW);
      color = "green";

    }

    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();

    unsigned long epochDay = epochTime / 86400; // 86400 seconds in a day
    int year = 1970;
    int month, day, hour, minute, second;
    while (epochDay >= (365 + ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))) {
      epochDay -= (365 + ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)));
      year++;
    }
    int daysInMonth[] = {31, 28 + ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (month = 0; epochDay >= daysInMonth[month]; month++) {
      epochDay -= daysInMonth[month];
    }

    day = epochDay + 1;
    hour = ((epochTime % 86400) / 3600)+1;
    minute = (epochTime % 3600) / 60;
    second = epochTime % 60;

    String timestamp = String(day) + "." + String(month + 1) + "." + String(year) + " " + String(hour) + ":" + String(minute) + ":" + String(second);

    String jsonData = "{\"color\":\"" + String(color) + "\", \"timestamp\":\"" + timestamp + "\"}";

    if (Firebase.pushJSON(firebaseData, "/colors", jsonData)) {
      Serial.println(firebaseData.dataPath() + " = " + firebaseData.pushName());
    } else {
      Serial.println(firebaseData.errorReason());
    }

    Serial.println(color + " " + timestamp);
    belt_ide = 1;
    digitalWrite(belt, HIGH);
    Serial.println("zase idem...");
  }
}
