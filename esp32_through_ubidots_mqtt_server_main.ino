/*
This project created by Amitave Rakhit
*/

/****************************************
   Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MCP3XXX.h>
#include <WiFiMulti.h>
#include "EEPROM.h"

//for eeprom address
int address = 0;

// for Temp Sensor comperatur
#define ONE_WIRE_BUS 26
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
double tempsum;
float temp;

// for Temp Sensor for normal
#define ONE_WIRE_BUS_normal 18
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire_normal(ONE_WIRE_BUS_normal);
DallasTemperature sensors_normal(&oneWire_normal);
double tempsum_normal;
float temp_normal;

// for Temp Sensor for deep
#define ONE_WIRE_BUS_deep 19
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire_deep(ONE_WIRE_BUS_deep);
DallasTemperature sensors_deep(&oneWire_deep);
double tempsum_deep;
float temp_deep;

// for mcp3008
MCP3008 adc;
#define cs 27
#define Clock 14
#define Miso 12
#define Mosi 13


//variable for current sensor
//#define acs712 34
int acspower = 5;
float amperage = 0.00;
float amp = 0.00;
float energy = 0.00;
double kilos = 0.00;
double kilossate = 0.00;
unsigned long last_time = 0;
unsigned long current_time = 0;

//current sample count
long lastsample = 0;
long samplesum = 0;
int sampleCount = 0;
float vpc = 4.8828125;

// variable for vibretion sensor
//int vbs = 4;
long viberation = 0;
long viberationsum = 0;

// variable for sound sensor
//int ss = A2;
float sound = 0;
long soundsum = 0;


//None blocking delay
unsigned long period = 61000;
unsigned long time_now = 0;

// Network and connection
#define WIFISSID "Nouvelle  vie" // Put your WifiSSID here
#define PASSWORD "Nopassword" // Put your wifi password here
#define TOKEN "BBFF-6kvjacOjg5Jul0w4JruYRpWTBqOC2E" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "Fridgemain" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
//it should be a random and unique ascii string and different from all other devices

/****************************************
   Define Constants
 ****************************************/
#define VARIABLE_LABEL "comp-temp" // Assing the variable label
#define VARIABLE_LABEL1 "current" // Assing the variable label
#define VARIABLE_LABEL2 "sound" // Assing the variable label
#define VARIABLE_LABEL3 "vibration" // Assing the variable label
#define VARIABLE_LABEL4 "watt" // Assing the variable label
#define VARIABLE_LABEL5 "kilowatt-hour" // Assing the variable label
#define VARIABLE_LABEL6 "normal-temp" // Assing the variable label
#define VARIABLE_LABEL7 "deep-temp" // Assing the variable label
#define DEVICE_LABEL "all-data-main" // Assig the device label

//#define relay 26 // Set the GPIO26 as RELAY

char mqttBroker[]  = "things.ubidots.com";
char payload[700];
char topic[150];
char topic1[150];
char topic2[150];
char topic3[150];
char topic4[150];
char topic5[150];
char topic6[150];
char topic7[150];

char topicSubscribe[100];
// Space to store values to send
char str_comp_temp[10];
char str_normal_temp[10];
char str_deep_temp[10];
char str_current[10];
char str_sound[10];
char str_vibration[10];
char str_watt[10];
char str_watt_hour[10];

/****************************************
   Auxiliar Functions
 ****************************************/
WiFiMulti WiFiMulti;
WiFiClient ubidots;
PubSubClient client(ubidots);



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
      client.subscribe(topicSubscribe);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}
//void callback(char* topic, byte* payload, unsigned int length) {
//  char p[length + 1];
//  memcpy(p, payload, length);
//  p[length] = NULL;
//  String message(p);
//  if (message == "0") {
//    digitalWrite(relay, LOW);
//  } else {
//    digitalWrite(relay, HIGH);
//  }
//
//  Serial.write(payload, length);
//  Serial.println();
//}

/****************************************
   Main Functions
 ****************************************/
void setup() {
  Serial.begin(115200);
  //acs power pin
  pinMode(acspower, OUTPUT);
  digitalWrite(acspower, LOW);
  //epprom setup
  EEPROM.begin(512); //Initialasing EEPROM
  WiFi.begin(WIFISSID, PASSWORD);
  // Assign the pin as INPUT
  //  pinMode(relay, OUTPUT);
  adc.begin(cs, Mosi, Miso, Clock);
  Serial.println();
  Serial.print("Wait for WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  //  client.setCallback(callback);

  //  sprintf(topicSubscribe, "/v1.6/devices/%s/%s/lv", DEVICE_LABEL, VARIABLE_LABEL_SUBSCRIBE);

  client.subscribe(topicSubscribe);
  //for ds18b20
  sensors.begin();
  sensors_normal.begin();
  sensors_deep.begin();

  // EEprom data remove
  //      EEPROM.writeDouble(address, kilossate);
  //      EEPROM.commit();
  // EEprom data read
  kilos = EEPROM.readDouble(address);
  Serial.print("EEPROM Save Data:");
  Serial.println(kilos);
}

void loop() {
  //    if (!client.connected()) {
  //      //    client.subscribe(topicSubscribe);
  //      reconnect();
  //    }

  // for take some sample

  if (millis() > lastsample + 1) {
    //take sample.
    samplesum += sq(adc.analogRead(4) - 511);
    viberationsum += adc.analogRead(2);
    soundsum += adc.analogRead(3);

    //    sensors.requestTemperatures();
    //    tempsum += sensors.getTempCByIndex(0);

    sampleCount++;
    lastsample = millis ();
  }

  if (sampleCount == 2000) {

    // for vibretion count
    viberation = viberationsum / sampleCount;

    //for sound count
    sound = soundsum / sampleCount;
    //    Serial.println(sound);
    //    float db = map(sound,0,600,25,100);
    float db = 29 * log10(sound);

    // temperature for comp
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);

    // temperature for Normal
    sensors_normal.requestTemperatures();
    temp_normal = sensors_normal.getTempCByIndex(0);

    // temperature for deep
    sensors_deep.requestTemperatures();
    temp_deep = sensors_deep.getTempCByIndex(0);

    // current measur
    float mean = samplesum / sampleCount;
    if (mean <= 3)
    {
      mean = 0;
    }
    //Serial.println(adc.analogRead(1));
    Serial.println(mean);
    float value = sqrt(mean);
    float mv = value * vpc;
    amperage = (mv / 48) * 1000; //66 for 30A  100 for 20A  185 for 5A
    amp = amperage / 1000.0;



    // watt measur
    energy = (amperage * .260) * 0.45;
    // tregar curret sensor
    if (energy < 40 && energy > 5)
    {

      digitalWrite(acspower, HIGH);
      energy = 0.0;
      delay(2000);
      digitalWrite(acspower, LOW);
      delay(100);

    }

    if ( energy >= 300)
    {

      digitalWrite(acspower, HIGH);
      energy = 0.00;
      delay(2000);
      digitalWrite(acspower, LOW);
      delay(100);

    }

    //kilowatt hour
    last_time = current_time;
    current_time = millis();

    kilos = kilos + (energy * ((current_time - last_time) / 60.0 / 60.0 / 1000.0 / 1000.0));

    /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
    dtostrf(amp, 4, 2, str_current);
    dtostrf(temp, 4, 2, str_comp_temp);
    dtostrf(temp_normal, 4, 2, str_normal_temp);
    dtostrf(temp_deep, 4, 2, str_deep_temp);
    dtostrf(db, 4, 2, str_sound);
    dtostrf(viberation, 4, 2, str_vibration);
    dtostrf(energy, 4, 2, str_watt);
    dtostrf(kilos, 4, 2, str_watt_hour);




    //  delay(30000);

    String dataMessage = String(" Miliamp: ") + String(amperage) + "," + String(" Watt: ") + String(energy) + "," + String(" KiloWatt Hour: ") + String(kilos) + "," + String(" ComT: ") + String(temp) + ", " + String(" NotmalT: ") + String(temp_normal) + ", " + String(" DeepT: ") + String(temp_deep) + ", " + String("Vibration: ") + String(viberation) + ", " + String(" sound: ") + String(db) + "\r\n";
    Serial.println(dataMessage);

    // write energy data
    EEPROM.writeDouble(address, kilos);
    EEPROM.commit();

    samplesum = 0;
    sampleCount = 0;
    viberationsum = 0;
    soundsum = 0;
    //    delay(600000);
  }


  //  if (millis() >= time_now + period) {
  if (millis() - time_now >= period) {

    if (!client.connected()) {
      //    client.subscribe(topicSubscribe);
      reconnect();
    }

    sprintf(topic6, "%s", ""); // Cleans the topic content
    sprintf(topic6, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL6); // Adds the variable label
    sprintf(payload, "%s {\"value\": %s}}", payload, str_normal_temp); // Adds the value
    Serial.println("Publishing data to Ubidots Cloud");
    client.publish(topic6, payload);
    delay(1500);
    //
    //    sprintf(topic1, "%s", ""); // Cleans the topic content
    //    sprintf(topic1, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
    //
    //    sprintf(payload, "%s", ""); // Cleans the payload
    //    sprintf(payload, "{\"%s\":", VARIABLE_LABEL1); // Adds the variable label
    //    sprintf(payload, "%s {\"value\": %s}}", payload, str_current); // Adds the value
    //    Serial.println("Publishing data to Ubidots Cloud");
    //    client.publish(topic1, payload);
    //    delay(1000);
    sprintf(topic7, "%s", ""); // Cleans the topic content
    sprintf(topic7, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL7); // Adds the variable label
    sprintf(payload, "%s {\"value\": %s}}", payload, str_deep_temp); // Adds the value
    Serial.println("Publishing data to Ubidots Cloud");
    client.publish(topic7, payload);

    delay(1500);


    sprintf(topic4, "%s", ""); // Cleans the topic content
    sprintf(topic4, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL4); // Adds the variable label
    sprintf(payload, "%s {\"value\": %s}}", payload, str_watt); // Adds the value
    Serial.println("Publishing data to Ubidots Cloud");
    client.publish(topic4, payload);
    delay(1500);


    sprintf(topic5, "%s", ""); // Cleans the topic content
    sprintf(topic5, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL5); // Adds the variable label
    sprintf(payload, "%s {\"value\": %s}}", payload, str_watt_hour); // Adds the value
    Serial.println("Publishing data to Ubidots Cloud");
    client.publish(topic5, payload);
    delay(1500);


    sprintf(topic, "%s", ""); // Cleans the topic content
    sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload content
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL); // Adds the variable label
    sprintf(payload, "%s {\"value\": %s}}", payload, str_comp_temp); // Adds the value
    //    sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
    Serial.println("Publishing data to Ubidots Cloud");
    client.publish(topic, payload);
    delay(1500);


    sprintf(topic2, "%s", ""); // Cleans the topic content
    sprintf(topic2, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL2); // Adds the variable label
    sprintf(payload, "%s {\"value\": %s}}", payload, str_sound); // Adds the value
    Serial.println("Publishing data to Ubidots Cloud");
    client.publish(topic2, payload);
    delay(1500);


    sprintf(topic3, "%s", ""); // Cleans the topic content
    sprintf(topic3, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL3); // Adds the variable label
    sprintf(payload, "%s {\"value\": %s}}", payload, str_vibration); // Adds the value
    Serial.println("Publishing data to Ubidots Cloud");
    client.publish(topic3, payload);
    delay(1500);
    client.loop();
    //    client.disconnect();
    //    time_now += period;
    time_now = millis();
  }

  //  delay(30000);
  //  ESP.deepSleep(600e6);
}
