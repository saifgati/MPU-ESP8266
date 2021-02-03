#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
boolean fall = false; //stores if a fall has occurred
boolean trigger1=false; //stores if first trigger (lower threshold) has occurred
boolean trigger2=false; //stores if second trigger (upper threshold) has occurred
boolean trigger3=false; //stores if third trigger (orientation change) has occurred
byte trigger1count=0; //stores the counts past since trigger 1 was set true
byte trigger2count=0; //stores the counts past since trigger 2 was set true
byte trigger3count=0; //stores the counts past since trigger 3 was set true
int angleChange=0;
char* ssid = "globalnet";
char* password = "00000000";
WiFiClient  client;

long myChannelNumber = 1289104;
const char myWriteAPIKey[]  = "73QHOAZHIKUWNMUN";

// Initialize our values
String myStatus = "";

void setup()
{
  WiFi.begin(ssid,password);
  Serial.begin(9600);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected\n");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

}

void loop()
{
 handleIndex();
 delay(5000);
}

void handleIndex()
{
  // Send a JSON-formatted request with key "type" and value "request"
  // then parse the JSON-formatted response with keys "gas" and "distance"
StaticJsonDocument<256> doc;
float accelerometer_x = 0, accelerometer_y = 0, accelerometer_z=0, temperature=0, gyro_x=0, gyro_y=0, gyro_z=0;
  // Sending the request
  doc["type"] = "request";
  serializeJson(doc,Serial);
  // Reading the response
  boolean messageReady = false;
  String message = "";
  while(messageReady == false) { // blocking but that's ok
    if(Serial.available()) {
      
      message = Serial.readString();
      messageReady = true;
    }
  }
  // Attempt to deserialize the JSON-formatted message
  DeserializationError error = deserializeJson(doc,message);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  accelerometer_x = doc["accelerometer_x"];
  accelerometer_y = doc["accelerometer_y"];
  accelerometer_z = doc["accelerometer_z"];
  temperature = doc["temperature"];
  gyro_x = doc["gyro_x"];
  gyro_y = doc["gyro_y"];
  gyro_z = doc["gyro_z"];


float  ax = (accelerometer_x-2050)/16384.00;
float  ay = (accelerometer_y-77)/16384.00;
float  az = (accelerometer_z-1947)/16384.00;
float  gx = (gyro_x+270)/131.07;
float  gy = (gyro_y-351)/131.07;
float  gz = (gyro_z+136)/131.07;
float Raw_Amp = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);
int Amp = Raw_Amp * 10; // Mulitiplied by 10 bcz values are between 0 to 1
if (Amp<=2 && trigger2==false)
{                          //if amplitude breaks lower threshold (0.4g)
   trigger1=true;
   Serial.println("TRIGGER 1 ACTIVATED");
   }
 if (trigger1==true)
{
   trigger1count++;
   
   if (Amp>=7)
{                         //if AM breaks upper threshold (3g)
     trigger2=true;
     Serial.println("TRIGGER 2 ACTIVATED");
     trigger1=false; trigger1count=0;
     }
 }
 if (trigger2==true)
{
   trigger2count++;
   angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5); Serial.println(angleChange);
   if (angleChange>=30 && angleChange<=400)
{                        //if orientation changes by between 80-100 degrees
     trigger3=true; trigger2=false; trigger2count=0;
     Serial.println(angleChange);
     Serial.println("TRIGGER 3 ACTIVATED");
       }
   }
 if (trigger3==true)
{
    trigger3count++;
    if (trigger3count>=5)
{ 
       angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
                       //delay(10);
       Serial.println(angleChange); 
       if ((angleChange>=0) && (angleChange<=10))
{                     //if orientation changes remains between 0-10 degrees
           fall=true; trigger3=false; trigger3count=0;
           Serial.println(angleChange);
             }
       else
{                    //user regained normal orientation
          trigger3=false; trigger3count=0;
          Serial.println("TRIGGER 3 DEACTIVATED");
       }
     }
  }
  int f = 0;
 if (fall==true)
{                  //in event of a fall detection
   Serial.println("Variation DETECTED");
   fall=false;
   f= 1;
   }

if ((accelerometer_x != 0) && (accelerometer_y != 0) )
{
  ThingSpeak.setField(1, accelerometer_x);
  ThingSpeak.setField(2, accelerometer_y);
  ThingSpeak.setField(3, accelerometer_z);
  ThingSpeak.setField(4, temperature);
  ThingSpeak.setField(5, gyro_x);
  ThingSpeak.setField(6, gyro_y);
  ThingSpeak.setField(7, gyro_z);
  ThingSpeak.setField(8, f);
  ThingSpeak.setStatus(myStatus);
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
 
  delay(4000); // Wait 4 seconds to update the channel again
}
  
}
