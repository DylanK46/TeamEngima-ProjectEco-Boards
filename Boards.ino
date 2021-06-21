/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp32-http-post-ifttt-thingspeak-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <HTTPClient.h>

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
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String temp = "";
      
      temp.concat("{"); 
      temp.concat("'LOCATION1': {"); 
      
        temp.concat("'Temperature':"); 
        temp.concat(random(0, 100)); 
        temp.concat(",");

        temp.concat("'Humidity':"); 
        temp.concat(random(0, 100)); 
        temp.concat(",");

        temp.concat("'CO2':"); 
        temp.concat(random(0, 100)); 

      temp.concat("}"); 
      temp.concat("}"); 
    

      // Send HTTP POST request
      int httpResponseCode = http.POST(temp);
      
      /*
      // If you need an HTTP request with a content type: application/json, use the following:
      http.addHeader("Content-Type", "application/json");
      // JSON data to send with HTTP POST
      String httpRequestData = "{\"value1\":\"" + String(random(40)) + "\",\"value2\":\"" + String(random(40)) + "\",\"value3\":\"" + String(random(40)) + "\"}";
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      */
     
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
