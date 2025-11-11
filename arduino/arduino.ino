#include "BMA250.h"
#include <Wire.h>

#include "SampleBuffer.h"

SampleBuffer samples;
BMA250 accel_sensor;
uint32_t last_micros;

void setup() {
  SerialUSB.begin(115200);
  Wire.begin();

  SerialUSB.print("Initializing BMA...");
  if (accel_sensor.begin(BMA250_range_2g, BMA250_update_time_8ms)) {
    SerialUSB.print("ERROR! NO BMA250 DETECTED!");
  }
}

void loop() {
  Sample sample = accel_sensor.read();
  samples.append(sample);

  waitForNextSample();
}

void waitForNextSample() {
  // Underflow is fine here as we only take the difference,
  // which is almost guaranteed to not overflow (70mins for a frame is crazy).
  uint32_t elapsed = micros() - last_micros;
  if (SAMPLE_MICROS > elapsed) {
    uint32_t remaining_milis = (SAMPLE_MICROS - elapsed) / 1000;
    delay(remaining_milis);
  }
  last_micros = micros();

  // SerialUSB.print("micros: ");
  // SerialUSB.println(elapsed);
}
