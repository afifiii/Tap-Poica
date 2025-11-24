#include "BMA250.h"
#include "Arduino.h"
#include <Wire.h>
#include <inttypes.h>

BMA250::BMA250() {}

int BMA250::begin(uint8_t range, uint8_t bw) {
  // Detect address
  I2Caddress = BMA250_I2CADDR;
  Wire.beginTransmission(I2Caddress);
  if (Wire.endTransmission()) {
    I2Caddress++;
    Wire.beginTransmission(I2Caddress);
    if (Wire.endTransmission()) {
      I2Caddress = 0;
      return -1;
    }
  }
  // Setup the range measurement setting
  Wire.beginTransmission(I2Caddress);
  Wire.write(0x0F);
  Wire.write(range);
  Wire.endTransmission();
  // Setup the bandwidth
  Wire.beginTransmission(I2Caddress);
  Wire.write(0x10);
  Wire.write(bw);
  Wire.endTransmission();
  // If the BMA250 is not found, nor connected correctly, these values will be
  // produced by the sensor
  Sample sample = read();
  if (sample.x == -1 && sample.y == -1 && sample.z == -1) {
    return -2;
  }
  return 0;
}

Sample BMA250::read() {
  // Set register index
  Wire.beginTransmission(I2Caddress);
  Wire.write(0x02);
  Wire.endTransmission();
  // Request seven data bytes
  Wire.requestFrom(I2Caddress, 7);
  // Receive acceleration measurements as 16 bit integers
  Sample sample;
  sample.x = (int16_t)Wire.read();
  sample.x |= (int16_t)Wire.read() << 8;
  sample.y = (int16_t)Wire.read();
  sample.y |= (int16_t)Wire.read() << 8;
  sample.z = (int16_t)Wire.read();
  sample.z |= (int16_t)Wire.read() << 8;
  // Discard temperature measurement
  Wire.read();
  return sample;
}
