//transmittor
#include <SPI.h>
#include "Wire.h" // This library allows you to communicate with I2C devices.
#include <VirtualWire.h>
const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
const int led_pin = 13;
const int transmit_pin = 12;

struct package
{
  int16_t temperature ;
  int16_t accelerometer_x ;
  int16_t accelerometer_y ;
  int16_t accelerometer_z ;
  int16_t gyro_x ;
  int16_t gyro_y ;
  int16_t gyro_z ;
};
typedef struct package Package;
Package data;


void setup() {
  // Initialise the IO and ISR
  vw_set_tx_pin(transmit_pin);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(500);       // Bits per sec
  pinMode(led_pin, OUTPUT);
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}
void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  data.accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  data.accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  data.accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  data.temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  data.gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  data.gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  data.gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
  
  // print out data
  Serial.println("Readings: ");
  Serial.print("aX = "); Serial.print(data.accelerometer_x);
  Serial.print(" | aY = "); Serial.print(data.accelerometer_y);
  Serial.print(" | aZ = "); Serial.print(data.accelerometer_z);
  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
  Serial.print(" | tmp = "); Serial.print(data.temperature/340.00+36.53);
  Serial.print(" | gX = "); Serial.print(data.gyro_x);
  Serial.print(" | gY = "); Serial.print(data.gyro_y);
  Serial.print(" | gZ = "); Serial.print(data.gyro_z);
  Serial.println("\n");
  

  digitalWrite(led_pin, HIGH); // Flash a light to show transmitting
  
  vw_send((uint8_t *)&data, sizeof(data));
  vw_wait_tx(); // Wait until the whole message is gone
   delay(2000);

  digitalWrite(led_pin, LOW);
  delay(2000);
 
 
}
