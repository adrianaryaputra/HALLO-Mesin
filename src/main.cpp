////////////
// DEPS
////////////

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>



////////////
// FUNCTION DECLARATION
////////////

void onFlagMQ(bool* FLAG, String COMMAND, String DATA2SEND);
void setup_wifi();
void setup_mqtt();
void reconnect();
void callback(char* topic, byte* message, unsigned int length);



////////////
// CONFIG
////////////

const char *SSID = "HALLO-MESIN";
const char *PWD = "12345678";

char *mqttServer = "192.168.3.200";
int mqttPort = 5003;

String PLANT = "HALLO";
String DEVICE_NAME = "MESIN_1";



////////////
// SETUP
////////////

WiFiClient espClient;
PubSubClient client(espClient);



////////////
// INTERRUPT FLAGS
////////////

int PIN_COUNT = 14;
int FILTER_COUNT = 200; //ms
int LAST_COUNT = 0; //millis
bool FLAG_COUNT = false;
void IRAM_ATTR setFlagCount() {
  if(millis() >= LAST_COUNT + FILTER_COUNT) {
    LAST_COUNT = millis();
    FLAG_COUNT = true;
  }
}

int PIN_MOTOR_EN = 34;
int FILTER_MOTOR_EN = 200; //ms
int LAST_MOTOR_EN = 0; //millis
bool FLAG_MOTOR_EN = false;
void IRAM_ATTR setFlagMotorEN() {
  if(millis() >= LAST_MOTOR_EN + FILTER_MOTOR_EN) {
    LAST_MOTOR_EN = millis();
    FLAG_MOTOR_EN = true;
  }
}



////////////
// PROGRAMS
////////////

void setup() {
  Serial.begin(9600);
  
  setup_wifi();
  setup_mqtt();

  pinMode(PIN_COUNT, INPUT_PULLUP);
  attachInterrupt(PIN_COUNT, setFlagCount, FALLING);

  pinMode(PIN_MOTOR_EN, INPUT_PULLUP);
  attachInterrupt(PIN_MOTOR_EN, setFlagMotorEN, CHANGE);
}

void loop() {
  // connect to wifi if disconnected
  if(!client.connected()) {
    reconnect();
  }

  // if connected to wifi
  else {
    onFlagMQ(&FLAG_COUNT, String("count"), String("{\"success\": true,\"payload\": 1}"));
    onFlagMQ(&FLAG_MOTOR_EN, String("motor"), String("{\"success\": true,\"payload\": "+ String(digitalRead(PIN_MOTOR_EN)) +"}"));
  }
}



////////////
// FUNCTION IMPLEMENTATION
////////////

void onFlagMQ(bool* FLAG, String COMMAND, String DATA2SEND) {
  if(*FLAG) {
    String topic = PLANT + "/" + DEVICE_NAME + "/" + COMMAND;
    Serial.println(topic);
    Serial.println(DATA2SEND);
    client.publish(topic.c_str(), DATA2SEND.c_str());
    *FLAG = false;
  }
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void setup_mqtt() {
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(DEVICE_NAME.c_str(), ("/"+DEVICE_NAME+"/will").c_str(), 2, false, "unexpected exit")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}