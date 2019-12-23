#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SPI.h>

#include <HX711.h>
#include "HockySensor.h"

HockySensor* hocky = new HockySensor();
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = D1;
const int LOADCELL_SCK_PIN = D2;

HX711 scalee;

bool blink = false;


// Replace with your network credentials
const char* ssid = "Uii";
const char* password = "12345678";

ESP8266WebServer server(80);   //instantiate server at port 80 (http port)

String templatepage = "<h1>hocky</h1>";


void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();

void setup(void){
  Serial.begin(9600);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  pinMode(D0, OUTPUT);

  // try connection
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // connection done !
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  // url routes
  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  // start server
  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");

  // start hocky
  // hocky->init();
  // Serial.println("Hocky started");

  scalee.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println("scale started");
}

void loop(void){
  server.handleClient();                    // Listen for HTTP requests from clients

  Serial.print("reading...   ");

  // long res = hocky->getData();                // read sensor data
  // Serial.print("Data read = ");
  // Serial.println(res);

  long data = 0;
  
  if (scalee.is_ready()) {
      data = scalee.read();
  }
  Serial.println(data);


  if(blink){
    digitalWrite(D0, HIGH);
  }else {
    digitalWrite(D0, LOW);
  }
  blink = !blink;
  delay(500);


}






/*
*    HTTP Handlers
*/


void handleRoot() {
  server.send(200, "text/plain", "Hello world!");   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}