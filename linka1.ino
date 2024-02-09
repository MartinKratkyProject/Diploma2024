#include <Firebase_Arduino_WiFiNINA.h>

#define FIREBASE_HOST "thirdproject-8258e-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "dN5PdgEWteAB6EM1c0W2ZhR3Buqkfe5Z4G9eHASK"
FirebaseData firebaseData;

/*#define WIFI_SSID "Telekom-738176"
#define WIFI_PASSWORD "drt23523b2dcddcp"
*/
#define WIFI_SSID "redmi"
#define WIFI_PASSWORD "pivojezivot"

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

void setup() {
  Serial.begin(9600);

  Serial.print("Attempting to connect to WiFi...");
  Serial.println();
  int wifi_status = WL_IDLE_STATUS;
  while (wifi_status != WL_CONNECTED) {
    wifi_status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);
  }
  Serial.print("successfully conneted to IP: ");
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

  Serial.begin(9600);

}

void loop() {
  digitalWrite(trig1, LOW);
  delayMicroseconds(2);
  digitalWrite(trig1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig1, LOW);

  duration1 = pulseIn(echo1, HIGH);
  distance1 = (duration1*.0343)/2;
  delay(100);
  if(distance1 < 10){
    belt_ide = 1;
    Serial.print("distance1 = ");
    Serial.println(distance1);
  }
  if(belt_ide == 1){
    digitalWrite(belt, HIGH);
    Serial.println("idem...");
  }

  digitalWrite(trig2, LOW);
  delayMicroseconds(2);
  digitalWrite(trig2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig2, LOW);
  duration2 = pulseIn(echo2, HIGH);
  distance2 = (duration2*.0343)/2;
  delay(100);
  
  Serial.println(distance1, distance2);
  if(distance2 < 10){
    Serial.print("distance2 = ");
    Serial.println(distance2);
    delay(2000);
    belt_ide = 0;
    digitalWrite(belt, LOW);
    Serial.println("stojim...");

    //red color
    digitalWrite(A2,LOW);
    digitalWrite(A3,LOW);
    frequencyRED = pulseIn(sensor, LOW);
    frequencyRED = map(frequencyRED, 25,72,255,0);
    Serial.print("R = ");
    Serial.print(frequencyRED);
    Serial.print("  ");
    delay(100);

    //green color
    digitalWrite(A2,HIGH);
    digitalWrite(A3,HIGH);
    frequencyGREEN = pulseIn(sensor, LOW);
    frequencyGREEN = map(frequencyGREEN, 30,90,255,0);
    Serial.print("G = ");
    Serial.print(frequencyGREEN);
    Serial.print("  ");
    delay(100);

    //blue color
    digitalWrite(A2,LOW);
    digitalWrite(A3,HIGH);
    frequencyBLUE = pulseIn(sensor, LOW);
    frequencyBLUE = map(frequencyBLUE, 25,70,255,0);
    Serial.print("B= ");
    Serial.print(frequencyBLUE);
    Serial.println("  ");
    delay(100);

    if (frequencyBLUE > frequencyRED & frequencyBLUE > frequencyGREEN){
    //modra high
    digitalWrite(red_led, LOW);
    digitalWrite(green_led, LOW);
    digitalWrite(blue_led, HIGH);
    color = "blue";
  
    }
    if (frequencyBLUE < frequencyRED & frequencyRED > frequencyGREEN){
    //red high
    digitalWrite(red_led, HIGH);
    digitalWrite(green_led, LOW);
    digitalWrite(blue_led, LOW);
    color = "red";
  
    }
    if (frequencyGREEN > frequencyRED & frequencyBLUE < frequencyGREEN){
    //zelena high
    digitalWrite(red_led, LOW);
    digitalWrite(green_led, HIGH);
    digitalWrite(blue_led, LOW);
    color = "green";  

    }
    String jsonData = "{\"color\":\"" + String(color) + "\"}";

    if (Firebase.pushJSON(firebaseData, "/colors", jsonData)) {
    Serial.println(firebaseData.dataPath() + " = "+ firebaseData.pushName());
    } else {
    Serial.println(firebaseData.errorReason());
    }
    
    Serial.println(color);
    belt_ide = 1;
    digitalWrite(belt, HIGH);
    Serial.println("zase idem...");
  }
}
