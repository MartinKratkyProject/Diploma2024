#include <Firebase_Arduino_WiFiNINA.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>

const int servoPin = 1; // Pin for servo control
Servo servo360; // Servo object

#define FIREBASE_HOST "myfirstproject-fa65c-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_AUTH "lQL8ehEa4s5sFAzNqaFI1VIE8OfwOm2oU9fGIoiF"
FirebaseData firebaseData;

#define WIFI_SSID "Telekom-738176"
#define WIFI_PASSWORD "drt23523b2dcddcp"

const int trigPin = 8;
const int echoPin = 9;

const int sensor = 3;
const int servoPIN = 2;
Servo myservo;

int frequencyRED = 0;
int frequencyGREEN = 0;
int frequencyBLUE = 0;

float duration, distance;
String color = "";

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

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

  pinMode(sensor, INPUT);
  myservo.attach(servoPIN);
  myservo.write(90);

  digitalWrite(A0, HIGH);
  digitalWrite(A1, LOW);
  servo360.attach(11);
  servo360.write(90);

  timeClient.begin();
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;
  Serial.print("Distance: ");
  Serial.println(distance);
  delay(500);

  if (distance < 10 & distance > 3) {
    //delay(2000);
    servo360.write(90); // Stop the servo
    getColorAndSendData(); // Get color data and send to Firebase
  } else {
    servo360.write(80); // Rotate the servo
  }
}




void getColorAndSendData() {
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

  if (frequencyBLUE > -200 && frequencyGREEN > -200 && frequencyRED > -240) {
    //biela high
    myservo.write(180); 
    color = "white";
  } else {
    if (frequencyBLUE > frequencyRED && frequencyBLUE > frequencyGREEN) {
      //modra high
      myservo.write(0); 
      color = "blue";
    }
    if (frequencyBLUE < frequencyRED && frequencyRED > frequencyGREEN) {
      //red high
      myservo.write(120); 
      color = "red";
    }
    if (frequencyGREEN > frequencyRED && frequencyBLUE < frequencyGREEN) {
      //zelena high
      myservo.write(60); 
      color = "green";
    }
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
  hour = ((epochTime % 86400) / 3600) + 1;
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
  servo360.write(80);
  delay(2000);
}
