#include <Servo.h> 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "";
const char* pwd      = "";
const char* mqtt_server = "vindaloo.local";
const char* mqtt_user = "vindaloo";
const char* mqtt_password = "vindaloo";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Servo myservo;
void connectToWifi() {
  Serial.print("Vindaloo connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pwd); // připojení k wifi
  while (WiFi.status() != WL_CONNECTED) //připojování k wifi tečkuje
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
  int i;
  for(i=0; i < 13; i++) { //připojeno, problíká
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
    String clientId = "VindalooLightsBlock-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password)) { //připojení k serveru
      Serial.println("Connected to the hub.");
      mqttClient.subscribe("vindaloo/light1");
      int i;
      for(i=0; i < 13; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);

        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
      }
      mqttClient.publish("vindaloo/info", "Modul světelný je připojen"); //oznámení problikem a v nodered
    } else {
      Serial.print("Connection unsuccesful, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Trying again in 5s...");
      delay(5000);
    }
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
void setup() 
{ 
  myservo.attach(0); 
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  pinMode(LED_BUILTIN, OUTPUT);
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);

  delay(10); 
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

    if(topic=="vindaloo/light1"){
    
      myservo.write(messageTemp.toInt()); //z noderedu přijde úhel otočení
  
    }
  }
void loop() {

   if (WiFi.status() != WL_CONNECTED) {
      connectToWifi();
  }
  if (!mqttClient.connected()) {
    mqttConnect();
  }
  mqttClient.loop();

  
} 
