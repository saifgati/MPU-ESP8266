//sender to esp
#include <ArduinoJson.h>


#include <VirtualWire.h>
#include<SoftwareSerial.h> //Included SoftwareSerial Library
String message = "";
bool messageReady = false;

const int receive_pin = 12;
const int led_pin = 13;

struct package
{
  int16_t temperature;
  int16_t accelerometer_x;
  int16_t accelerometer_y ;
  int16_t accelerometer_z;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z ;
 
};
typedef struct package Package;
Package data;



void setup()
{
      // Initialise the IO and ISR
    vw_set_rx_pin(receive_pin);
    vw_setup(500);   // Bits per sec
    vw_rx_start();       // Start the receiver PLL running
    Serial.begin(9600);
}

void loop()
{
    uint8_t buf[sizeof(data)];
    uint8_t buflen = sizeof(data);

if (vw_have_message())  // Is there a packet for us? 
  {
    vw_get_message(buf, &buflen);
    memcpy(&data,&buf,buflen);

  Serial.print("\nPackage:");
  Serial.println(); 
  Serial.print("aX = "); Serial.print(data.accelerometer_x);
  Serial.print(" | aY = "); Serial.print(data.accelerometer_y);
  Serial.print(" | aZ = "); Serial.print(data.accelerometer_z);
  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
  Serial.print(" | tmp = "); Serial.print(data.temperature/340.00+36.53);
  Serial.print(" | gX = "); Serial.print(data.gyro_x);
  Serial.print(" | gY = "); Serial.print(data.gyro_y);
  Serial.print(" | gZ = "); Serial.print(data.gyro_z);
  Serial.println();
  }
// Monitor serial communication
  while(Serial.available()) {
    message = Serial.readString();
    messageReady = true;
  }
  // Only process message if there's one
  if(messageReady) {
    // The only messages we'll parse will be formatted in JSON
    StaticJsonDocument<192> doc;    // Attempt to deserialize the message
    DeserializationError error = deserializeJson(doc,message);
    if(error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      messageReady = false;
      return;
    }
    if(doc["type"] == "request") {
      doc["type"] = "response";
      // Get data from analog sensors
      doc["accelerometer_x"] = data.accelerometer_x;
      doc["accelerometer_y"] = data.accelerometer_y;
      doc["accelerometer_z"] = data.accelerometer_z;
      doc["temperature"] = data.temperature/340.00+36.53;
      doc["gyro_x"] = data.gyro_x;
      doc["gyro_y"] = data.gyro_y;
      doc["gyro_z"] = data.gyro_z;
      serializeJson(doc,Serial);
    }
    messageReady = false;
  }
}
