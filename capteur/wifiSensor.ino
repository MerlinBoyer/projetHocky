#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "AgvControl.h"


/*------------------- CONSTS ----------------------*/

// Replace with your network credentials
const char* ssid = "aiosmartbutton";
const char* password = "aiosmartbutton";

/*const int Lred = 13;
const int Lblue = 12;
const int Lyellow = 14;*/

const int Y21 = 13; // Lred
const int X92 = 12; // Lgreen
const int X93 = 14; // Lyellow
const int Y22 = 15; // Lblue
const int X82 = 12; // Lwhite
  
AgvControl* agvController = new AgvControl();

ESP8266WebServer server(80);   //instantiate server at port 80 (http port)

//HTML content
String templatepage = "<h1>AIO smartButton - recepteur</h1>";

String message = "<p> hello! </p>";
String page = "";


/*-------------------FUNCTIONS----------------------*/


String createSimpleJson(String key, String value)
{
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder[key] = value;
  char JSONmessageBuffer[300];
  JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  return String(JSONmessageBuffer);
}

String createArrayJson(String key, int* valuesArray)
{
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JsonArray& values = JSONencoder.createNestedArray( key );
  for (int i = 0; i < sizeof(valuesArray) + 1 ; i++) {
    values.add( valuesArray[i] );
  }
  char JSONmessageBuffer[300];
  JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  return String(JSONmessageBuffer);
}


String getValueFromJson(String key)
{
   DynamicJsonBuffer jsonBuffer( 256 );
   JsonObject& root = jsonBuffer.parseObject( server.arg("plain") );
   if ( !root.containsKey(key) ){
     Serial.println("Json key " + key + " not found");
     server.send(400, "text/plain", "400: Bad request, cannot get json key : " + key);
     return "";
   }
   return root[ key ];
}

int requestHasNoBody() {
  if (server.hasArg("plain") == false) {
    Serial.println(">> no body received");
    server.send(400, "text/plain", "400: Bad request, request has no body. impossible to get arguments");
    return 1;
  } else {
    return 0;
  }
}

String dumpAGV() {
  String str = "\nAgv status : \n";
  str += "  current course : " + String(agvController->getCurrentCourse()) + "\n";
  str += "  nb courses programmed : " + String(agvController->getNbCoursesProgrammed()) + "\n";
  str += "  is busy ? : " + String(agvController->isBusy()) + "\n";
  str += "  programmed courses : " + String(agvController->getProgrammedCourse(0)) + " ";
  str += String(agvController->getProgrammedCourse(1)) + " ";
  str += String(agvController->getProgrammedCourse(2)) + " ";
  str += String(agvController->getProgrammedCourse(3)) + " ";
  str += String(agvController->getProgrammedCourse(4)) + " ";
  return str;
}

/*-------------------HANDLE REQUESTS----------------------*/

void handlegetCurrentCourse() {
  String json = createSimpleJson("currentCourse", String(agvController->getCurrentCourse()) );
  server.send(200, "application/json", json);
}

/*  // TODO //
  void handlegetBattery() {
  }
*/

void handlegetgetAllProgrammedCourses() {
  int courses[MAX_PROGRAMMED_COURSES] = {0};
  agvController->getAllProgrammedCourses( courses );
  String json = createArrayJson("courses", courses);
  server.send(200, "application/json", json);
}

void handlegetAllCourses() {
  int courses[MAX_COURSES] = {0};
  agvController->getAllCourses( courses );
  String json = createArrayJson("courses", courses);
  server.send(200, "application/json", json);
}

void handleSetCourse() {
  // if POST has no body, return error
  if ( requestHasNoBody() ) {
    return;
  }

  String course = getValueFromJson("course");
  Serial.print(">> course received : ");
  Serial.println(course);
  if ( course == "") {
    return;
  }
  int res = agvController->addCourse( course.toInt() );
  if ( res == 0) {
    Serial.println(">>  nb_courses_programmed == MAX_PROGRAMMED_COURSES");
  } else {
    Serial.println(">> parcours " + course + "saved");
  }
  Serial.println( dumpAGV() );

  String answer = createSimpleJson("currentCourse", String(agvController->getCurrentCourse()) );
  server.send(200, "application/json", answer);
}

void handledeleteCourse() {
  if ( requestHasNoBody() ) {
    return;
  }
  String id_to_delete = getValueFromJson("position");
  
  Serial.println(">>deleting requested at position " + id_to_delete);
  int deleted = agvController->deleteCourse( id_to_delete.toInt() );
  if ( deleted ) {
    Serial.println("deleting done!");
    server.send(200, "text/plain", "200: ok, deleting done");
  } else {
    Serial.println("deleting impossible");
    server.send(400, "text/plain", "400: Bad request, deleting not possible");
  }
  Serial.println( dumpAGV() );
  return;
}

void handledeleteAllCourses() {
  Serial.println(">>delete all programmed courses ");
  agvController->deleteAllCourses();
  server.send(200, "text/plain", "200: OK, all programmed courses deleted");
}


void handleFreeAGV() {
  Serial.println(">>free AGV ");
  agvController->setAgvState( 0 );
  Serial.println( dumpAGV() );
  server.send(200, "text/plain", "200: OK, agv not busy anymore");
}

void handleSwitchCourse() {
  Serial.println(">>SwitchCourse ");
  DynamicJsonBuffer jsonBuffer( 256 );
  JsonObject& root = jsonBuffer.parseObject( server.arg("plain") );
  if ( !root.containsKey("position") || !root.containsKey("move") ){
    Serial.println("Json key not found");
    server.send(400, "text/plain", "400: Bad request, cannot get json key");
    return;
  }
  int index =  root[ "position" ];
  int movement =  root[ "move" ];
  int result = agvController->switchCourse(index, movement);
  if (result == 1) {
    server.send(200, "text/plain", "200: OK, sequence started");
  }
  else {
     Serial.println("switching index impossible");
      server.send(400, "text/plain", "400: Bad request, interchanging position not possible");
    }
}

// -----------------------------------------------------------------------------------
// Demo sequence leds 

void handleGetSequence() {
  Serial.println(">>sequence ");
  programmAGV2();
  server.send(200, "text/plain", "200: OK, sequence started");
}

// -----------------------------------------------------------------------------------

/*--------------------MAIN-----------------------*/
void programmAGV(int course){
  /*if ( course == 1){
      digitalWrite(Y21, HIGH);
      delay(500);
      digitalWrite(Y21, LOW);
    }
    if ( course == 2){
      digitalWrite(X92, HIGH);
      delay(500);
      digitalWrite(X92, LOW);
    }
    if ( course == 3){
      digitalWrite(X93, HIGH);
      delay(500);
      digitalWrite(X93, LOW);
    }*/
    agvController->setAgvState( 1 ); //set AGV busy
}

void programmAGV2(){

    digitalWrite(X92, HIGH);
    digitalWrite(Y21, HIGH);    
    delay(500);    
    digitalWrite(Y21, LOW);
    delay(1000);
    digitalWrite(X92, LOW); 
    digitalWrite(X93, HIGH); 
    delay(500);
    digitalWrite(Y21, LOW);
    delay(500);
    digitalWrite(Y22, HIGH);
    delay(500);
    digitalWrite(Y22, HIGH);
    delay(500); 
    digitalWrite(X92, LOW);
    digitalWrite(X93, LOW);
    delay(500);
    digitalWrite(Y22, LOW);
    delay(500);
    digitalWrite(X82, HIGH);
    delay(500);
    digitalWrite(X82, LOW);
    
    delay(3000);
    
    //agvController->setAgvState( 1 ); //set AGV busy
}

void setup(void) {

  // setup leds
  pinMode(Y21, OUTPUT);
  pinMode(X92, OUTPUT);
  pinMode(X93, OUTPUT);
  pinMode(Y22, OUTPUT);
  pinMode(X82, OUTPUT);
  
  //init wifi server
  page = templatepage;

  delay(1000);
  Serial.begin(115200);
  WiFi.softAP(ssid, password); //begin WiFi access point
  Serial.println("");
  server.on("/", []() {
    server.send(200, "text/html", page);
  });

  //// GETTERS
  
  //------------------------ GETTERS ---------------------------------------------------------------
  
  server.on("/getCurrentCourse", handlegetCurrentCourse);
  
  server.on("/getAllProgrammedCourses", handlegetgetAllProgrammedCourses);

  server.on("/getAllCourses", handlegetAllCourses);

  // TODO //
  // server.on("/getBattery", handlegetBattery);
  
  //------------------------ SETTERS ---------------------------------------------------------------
  
  server.on("/deleteCourse", handledeleteCourse);

  server.on("/deleteAllCourses", handledeleteAllCourses);

  server.on("/setCourse", handleSetCourse);

  server.on("/freeAGV", handleFreeAGV);

  server.on("/switchCourse", handleSwitchCourse);

  //------------------------ DEBUG ---------------------------------------------------------------
  
  server.on("/getSequence", handleGetSequence);
    
  server.on("/agvdump", [](){
    server.send(200, "text/html", dumpAGV() );
  });

  server.begin();
  Serial.println("Web server started!");
}

void loop(void){
  server.handleClient();
  delay(1);
  if ( agvController->isReadyToGo() ) {
    Serial.println(">>ready to go!");
    Serial.println( dumpAGV() );
    agvController->goOnNextCourse();
    int course = agvController->getCurrentCourse();
    Serial.print(">>new course started : ");
    Serial.println(course);
    programmAGV( course );
    Serial.println( dumpAGV() );
    delay(10);
  }
  delay(1);
}
