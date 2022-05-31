#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
const int dOut = 5;
const int dOutCO = 0;
const int aOut = A0;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
const char* ssid     = "";
const char* pwd      = "";
const char* mqtt_server = "vindaloo.local";
const char* mqtt_user = "vindaloo";
const char* mqtt_password = "vindaloo";
unsigned long lastPublish = 0;

void connectToWifi() {
  Serial.print("Vindaloo connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pwd); // připojení k WiFi
  while (WiFi.status() != WL_CONNECTED) //oznámit připojení problíkáním
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
  int i;
  for(i=0; i < 13; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);

    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
  Serial.println("Connection successful!");
  Serial.println("Vindaloo Home Automation Ready");
  Serial.print("Your IP address is: ");
  Serial.println(WiFi.localIP());
}

void mqttConnect() {
  while (!mqttClient.connected()) {
    Serial.print("Vindaloo is connecting to the hub, please wait.");
    String clientId = "VindalooSensorBlock-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password)) { //po připojení k serveru
      Serial.println("Connected to the hub.");
      mqttClient.subscribe("vindaloo/sensor");
      int i;
      for(i=0; i < 13; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);

        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
      }
      mqttClient.publish("vindaloo/info", "Modul senzorický je připojen"); //oznámit aktivaci v nodered
    } else {
      Serial.print("Connection unsuccesful, rc="); //po neúspěchu
      Serial.print(mqttClient.state());
      Serial.println(" Trying again in 5s...");
      delay(5000);
    }
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
void setup() {
  Serial.begin(9600);
  dht.begin();
  WiFi.mode(WIFI_STA);
  pinMode(LED_BUILTIN, OUTPUT); //nastavení pinů
  pinMode(dOut, INPUT);
  pinMode(aOut, INPUT);
  pinMode(dOutCO, INPUT);
  mqttClient.setServer(mqtt_server, 1883);
  delay(10);
}

void loop() {
   if (WiFi.status() != WL_CONNECTED) { //pokud se odpojí od wifi, tak se snaží znovu připojit
      connectToWifi();
  }
  if (!mqttClient.connected()) {
    mqttConnect();
  }
  mqttClient.loop();
  char hum[8];
  char temp[8];
  char flam[8];
  mqttClient.publish("vindaloo/humidity", dtostrf(dht.readHumidity(), 6, 2, hum)); //odesílá vlhkost
  mqttClient.publish("vindaloo/temperature", dtostrf(dht.readTemperature(), 6, 2, temp)); //odesílá teplotu
  mqttClient.publish("vindaloo/flammablegas",itoa(map(analogRead(aOut), 0, 1023, 0, 100), flam, 10)); //odesílá plyny
  if(digitalRead(dOutCO)) //odesílá jestli je oxid uhelnatý ok
  {
    mqttClient.publish("vindaloo/carbonmonoxideok", "danger");
  }
  else  
  {
    mqttClient.publish("vindaloo/carbonmonoxideok", "ok");
  }
  if(digitalRead(dOut)) //odesílá jestli jsou plyny ok
  {
    mqttClient.publish("vindaloo/flammableok", "danger");
  }
  else  
  {
    mqttClient.publish("vindaloo/flammableok", "ok");
  }
  delay(2000);
}
