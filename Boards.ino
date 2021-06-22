/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp32-http-post-ifttt-thingspeak-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

StaticJsonBuffer<200> jsonBuffer;

const char* ssid = "SKY8E685";
const char* password = "QFEEEUAV";

const char* serverName = "https://dev-test.projecteco.ml/api/v1/rest/input/sensors/";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");

  // Random seed is a number used to initialize a pseudorandom number generator
  randomSeed(analogRead(33));
}

void loop() {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/json");

      // -------------------------------- //

      // Make a json memory buffer
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();

      // Build your own object tree in memory to store the data you want to send in the request
      
      //make an array under location1
      JsonObject& data = root.createNestedObject("LOCATION1");
      data.set("Temperature", random(0, 100));
      data.set("Humidity", random(0, 100));
      data.set("CO2", random(0, 100));
      
      // Generate the JSON string
      root.printTo(Serial);

      // -----------------------------------// 
    
      String output;
      serializeJson(root, output);
  
      // Send HTTP POST request
      int httpResponseCode = http.POST(output);
      
      /*
      // If you need an HTTP request with a content type: application/json, use the following:
      http.addHeader("Content-Type", "application/json");
      // JSON data to send with HTTP POST
      String httpRequestData = "{\"value1\":\"" + String(random(40)) + "\",\"value2\":\"" + String(random(40)) + "\",\"value3\":\"" + String(random(40)) + "\"}";
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      */

      Serial.print(temp);
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    delay(2000);
}
