// Import Libraries
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "DHT.h"
#include "Adafruit_SGP30.h"
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Define blue led pin
#define ONBOARD_LED  2

// Define DHT11 Data pin as 4
#define DHT11PIN 4
DHT dht(DHT11PIN, DHT11);

// Define Adafruit sensor as sgp
Adafruit_SGP30 sgp;

// define ssid and password of wifi
const char* ssid = "";
const char* password = "";

// define board location 
const String boardloc = "";

// define token
const String authtoken ° "";

// define server to send http requests to
const char* serverName = "https://dev-test.projecteco.ml/api/v1/rest/input/sensors/";

// define the post interval in ms
const int postinterval = 1000;

// Some random code here, IDK what it does;
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature));
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}
// contact IT support at 19songv@harrowschool.org.uk for more information






// On setup,
void setup() {

  // Init wifi mode
  WiFi.mode(WIFI_STA);
  Serial.println("WIFI   > Initialising");
  
  // Start Serial at 115200 baud
  Serial.begin(115200);
  Serial.println("SERIAL > Started Serial");

  // Start DHT11 Sensor
  dht.begin();
  Serial.println("DHT    > Sensor Initiated");
   
  // Connect to the wifi
  WiFi.begin(ssid, password);
  Serial.println("WIFI   > Attempting Connection");

  // While waiting to connect to wifi, print dots
  Serial.print("WIFI   > Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  // print newline, print ip
  Serial.println("WIFI   > Connected");
  Serial.print("WIFI   > IP Address: ");
  Serial.println(WiFi.localIP());

  //print newline macaddress
  Serial.print("WIFI   > MAC Address:  ");
  Serial.println(WiFi.macAddress());

  Serial.println("SGP30  > Searching for Sensor");
  if (! sgp.begin()){
    Serial.println("SGP30  > Sensor not found");
    while (1);
  }
  
  Serial.print("SGP30  > Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  pinMode(ONBOARD_LED,OUTPUT);
  Serial.println("LEDS   > Leds initialised");

  // Initialise OTA Updates
  String hostname_str = "TEAMENIGMA - " + boardloc;
  char* hostname_char_arr = &hostname_str[0];
  ArduinoOTA.setHostname(hostname_char_arr);
  ArduinoOTA.setPassword("");
  Serial.println("OTA    > Hostname and Auth initialised");
  
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("OTA    > Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("OTA    > Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("OTA    > Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("OTA    > Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA    > Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("OTA    > End Failed");
    });

  ArduinoOTA.begin();
  Serial.println("OTA    > Server Started");
  
}





int counter = 0;
long lastMsg = 0;


// repeat:
void loop() {
     ArduinoOTA.handle();
  
    // If the WIFI is working:
    if(WiFi.status()== WL_CONNECTED){
      
    
      long now = millis();
      if (now - lastMsg > postinterval) {
        
        // Timing Stuff
        lastMsg = now;
  
        // Test if Measurement Failed 
        if (! sgp.IAQmeasure()) {
          Serial.println("SGP30  > Measurement failed");
          return;
        }
  
        // Test if raw measurement failed
        if (! sgp.IAQmeasureRaw()) {
          Serial.println("SGP30  > Raw Measurement failed");
          return;
        }
  
        // Init JSON file and http request
        DynamicJsonDocument doc(1024);
        HTTPClient http;
        
        // Tell http library to send request to server defined in variable "serverName"
        http.begin(serverName);
        
        // Specify content-type header
        http.addHeader("Content-Type", "application/json");
  
        // chuck the data in the JSON file
        doc[boardloc]["Temperature"] = dht.readTemperature();
        doc[boardloc]["Humidity"] = dht.readHumidity();
        doc[boardloc]["TVOC"] = sgp.TVOC;
        doc[boardloc]["eCO2"] = sgp.eCO2;
        doc[boardloc]["rawH2"] = sgp.rawH2;
        doc[boardloc]["rawEthanol"] = sgp.rawEthanol;
        doc["token"] = authtoken;

        // chuck the data to serial monitor
        Serial.print("Temperature: "); Serial.println(dht.readTemperature());
        Serial.print("Humidity: "); Serial.println(dht.readHumidity());
        Serial.print("TVOC: "); Serial.println(sgp.TVOC);
        Serial.print("eCO2: "); Serial.println(sgp.eCO2);
        Serial.print("rawH2: "); Serial.println(sgp.rawH2);
        Serial.print("rawEthanol: "); Serial.println(sgp.rawEthanol);
        Serial.println();
  
        // Convert that to a string called "json"
        String json;
        serializeJson(doc, json);
  
        // Send HTTP POST request with that string
        int httpResponseCode = http.POST(json);
  
        // Print response to Serial
        Serial.print("WIFI   > Data Sent - HTTP Response code: ");
        Serial.println(httpResponseCode);
          
        // Free resources by ending request
        http.end(); 

        // wait predefined interval
        digitalWrite(ONBOARD_LED,HIGH);
        delay(100);
        digitalWrite(ONBOARD_LED,LOW);
        counter++;
        }
      } else {
        // if no wifi, print disconnected.
        Serial.println("WIFI   > Disconnected");
      }    
  }
