#include "HockySensor.h"
#include <Arduino.h>

#include <HX711.h>

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = (int) MISO;
const int LOADCELL_SCK_PIN = (int) MOSI;

HX711 scale;


HockySensor::HockySensor(){

}

HockySensor::~HockySensor(){
    
}

void HockySensor::init(){
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale();  // Start scale
    scale.tare();       // Reset scale to zero
    return;
}



long HockySensor::getData(){
    long data = 0;
    // if (scale.is_ready()) {
    //     data = scale.read();
    // }

    float current_weight=scale.get_units(20);  // get average of 20 scale readings
    float scale_factor=(current_weight/0.145);  // divide the result by a known weight
    Serial.println(current_weight);  // Print the scale factor to use
    Serial.println(scale_factor);  // Print the scale factor to use

    return data;
}