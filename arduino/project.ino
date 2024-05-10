#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

/************************* DHT Sensor Setup *********************************/

#define DHTPIN   D2     // Digital pin connected to the DHT sensor
#define DHTTYPE  DHT11  // Type of DHT Sensor, DHT11 or DHT22

DHT dht(DHTPIN, DHTTYPE);

#define pumpout   D3     // Connect LED on pin D3

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "FREE"   
#define WLAN_PASS       "12345679"   

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                                 // use 8883 for SSL
#define AIO_USERNAME    "hoanghao2208"                        
#define AIO_KEY         ""   

/************ Global State ******************/


WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/cambien1"); 
Adafruit_MQTT_Publish hum = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/cambien3");

// controlling pump
Adafruit_MQTT_Subscribe pumpin = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/nutnhan2");

void MQTT_connect();

float lastHumidity = -1;
float lastTemperature = -1;
void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(pumpout, OUTPUT);
  Serial.println(F("Adafruit MQTT"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&pumpin);
  dht.begin();
}

void loop() {
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here


  Adafruit_MQTT_Subscribe *subscription;

  // Wait 2000 milliseconds, while we wait for data from subscription feed. After this wait, next code will be executed
  while ((subscription = mqtt.readSubscription(200))) {
    if (subscription == &pumpin) {
      Serial.print(F("Got Pump: "));
      char *pumpstate = (char *)pumpin.lastread;
      Serial.println(pumpstate);
      String message = String(pumpstate);
      message.trim();
      if (message == "1") {
digitalWrite(pumpout, HIGH);
        Serial.println("PUMP ON");
      }
      if (message == "0") {
        digitalWrite(pumpout, LOW);
        Serial.println("PUMP OFF");
      }
    }
  }

  // DHT11 Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));


  // Now we can publish stuff!
  if(lastHumidity != h){
    Serial.print(F("\nSending Humidity val "));
    if (! hum.publish(h)) {
        Serial.println(F("Failed"));
    } else {
        Serial.println(F("OK!"));
    }
    lastHumidity = h;
  }
  if(lastTemperature != t){
    Serial.print(F("\nSending Temperature val "));
    if (! temp.publish(t)) {
        Serial.println(F("Failed"));
    } else {
        Serial.println(F("OK!"));
    }
    lastTemperature = t;
  }


}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}