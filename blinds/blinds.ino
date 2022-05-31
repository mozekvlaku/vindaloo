#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

const char* ssid     = "";
const char* pwd      = "";
const char* mqtt_server = "vindaloo.local";
const char* mqtt_user = "vindaloo";
const char* mqtt_password = "vindaloo";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

int downSwitch = 0;
int upSwitch = 2;

unsigned long lastPublish = 0;
uint8_t servonum = 0;
#define SERVOMIN  150
#define SERVOMAX  600
#define USMIN  600
#define USMAX  2400 
#define SERVO_FREQ 50
void positionServo(int pos) {
  double pwm0 = map(pos, 0, 180, SERVOMIN, SERVOMAX); //převádí úhel na servo pwm
  pwm.setPWM(0,0,pwm0); //posílá info do serva 0
}
void connectToWifi() {
  Serial.print("Vindaloo connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pwd);
  while (WiFi.status() != WL_CONNECTED) //připojování k wifi
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
  int i;
  for(i=0; i < 13; i++) { //připojeno - probliká
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
    String clientId = "VindalooBlindsBlock-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password)) { //připojení k mosquitto serveru
      Serial.println("Connected to the hub.");
      mqttClient.subscribe("vindaloo/blinds"); //přihlášení ke zprávám
      int i;
      for(i=0; i < 13; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);

        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
      }
      mqttClient.publish("vindaloo/info", "Modul žaluzií je připojen"); //oznámení problikem a v NodeRedu
    } else {
      Serial.print("Connection unsuccesful, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Trying again in 5s...");
      delay(5000);
    }
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
void callback(String topic, byte* message, unsigned int length){ //reakce na příchozí zprávy
  Serial.print("Function ");
  Serial.print(topic);
  Serial.print(" got message: ");
  String messageTemp;
  int i;
  
  for(int i=0; i<length; i++){
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
    }
  Serial.println();  

  if(topic=="vindaloo/blinds"){
    Serial.print("The blinds are changing the state: ");
    if(messageTemp=="up"){ //vysouvání žaluzií
      Serial.println("Blinds going up");
      positionServo(90);
      
      
    }
    if(messageTemp=="down"){ //zasouvání žaluzií
      positionServo(0);
      Serial.println("Blinds going down");
     
      
      }
    
    if(messageTemp=="stop"){ //Stop žaluzií
       positionServo(180);
       digitalWrite(LED_BUILTIN, HIGH);
       Serial.println("Blinds stopped");
      }
    }
  }



void setup() {
  pinMode(upSwitch, INPUT_PULLUP);
  pinMode(downSwitch, INPUT_PULLUP);
  Serial.begin(9600);
  pwm.begin();
  WiFi.mode(WIFI_STA);
  pinMode(LED_BUILTIN, OUTPUT);
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);



  delay(10); 
}
int state = 0;
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
      connectToWifi();
  }
  if (!mqttClient.connected()) {
    mqttConnect();
  }
  mqttClient.loop();
  
  if(digitalRead(downSwitch) == 0 && state != 1) { //pokud se sepne spínač, tak se žaluzie zastaví
      positionServo(180);
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("Blinds stopped down");
      state = 1;
      mqttClient.publish("vindaloo/blindso", "stop");
  }

  if(digitalRead(upSwitch) == 0 && state != 2) {
      positionServo(180);
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("Blinds stopped up");
      state = 2;
      mqttClient.publish("vindaloo/blindso", "stop");
  }
}
