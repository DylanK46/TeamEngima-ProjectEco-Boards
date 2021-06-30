// Import Libraries
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "DHT.h"
#include "Adafruit_SGP30.h"

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
  
}






// repeat:
void loop() {
  
    // If the WIFI is working:
    if(WiFi.status()== WL_CONNECTED){

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
      doc["LOCATION1"]["Temperature"] = dht.readTemperature();
      doc["LOCATION1"]["Humidity"] = dht.readHumidity();
      doc["LOCATION1"]["TVOC"] = sgp.TVOC;
      doc["LOCATION1"]["eCO2"] = sgp.eCO2;
      doc["LOCATION1"]["rawH2"] = sgp.rawH2;
      doc["LOCATION1"]["rawEthanol"] = sgp.rawEthanol;

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
    }
    else {

      // if no wifi, print disconnected.
      Serial.println("WIFI   > Disconnected");
    }

    // wait predefined interval
    digitalWrite(ONBOARD_LED,HIGH);
    delay(100);
    digitalWrite(ONBOARD_LED,LOW);
    delay(postinterval-100);
    
}
