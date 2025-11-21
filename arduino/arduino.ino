#include "BMA250.h"
#include <Wire.h>

#include "DisplayManager.h"
#include "SampleBuffer.h"

#define TAP_THRESH 5600.0
#define TAP_DEBOUNCE 60000 // microseconds
#define SHAKE_THRESH 510.0
#define SHAKE_DEBOUNCE 130000 // microseconds

#define VIBRATE_PIN 6
#define VIBRATE_PIN_ACTIVE HIGH
#define VIBRATE_PIN_INACTIVE LOW

char sprintbuff[100];

//
// Global Objects
//
TinyScreen display = TinyScreen(TinyScreenDefault);
Timer _timer(display);
MenuManager menuManager(_timer, display);
BLEManager bleManager;
DisplayManager displayManager(display, _timer, menuManager, bleManager);
SampleBuffer samples;
BMA250 accel_sensor;
uint32_t clock_micros;
uint32_t last_tap = 0, last_shake = 0;
bool is_shaking = false;

typedef struct __attribute__((packed)) _GamePacket {
  uint8_t tapCounter : 4;
  uint8_t shakeCounter : 4;
} GamePacket_t;

void setup() {
  SerialMonitorInterface.begin(115200);
  Wire.begin();
  pinMode(VIBRATE_PIN, OUTPUT);
  digitalWrite(VIBRATE_PIN, VIBRATE_PIN_INACTIVE);

  PRINTF("Initializing BMA...");
  if (accel_sensor.begin(BMA250_range_2g, BMA250_update_time_4ms)) {
    PRINTF("ERROR! NO BMA250 DETECTED!");
  }

  clock_micros = micros();
  displayManager.begin();
  bleManager.begin();
}

void loop() {
  Sample sample = accel_sensor.read();
  samples.append(sample);
  if (clock_micros - last_tap > TAP_DEBOUNCE) {
    detectTap();
  }
  detectShake();

  waitForNextSample();

  bleManager.update();
  displayManager.update();
}

void detectTap() {
  float val = samples.get(samples.len() - 1) - samples.mean();
  if (val > TAP_THRESH || val < -TAP_THRESH) {
    PRINTF("Tap\n");
    last_tap = clock_micros;
  }
}

void detectShake() {
  float val = samples.std_dev();
  bool still_shaking = (val > SHAKE_THRESH);
  if (still_shaking) {
    last_shake = clock_micros;
  }
  if (is_shaking == still_shaking) {
    return;
  }

  if (is_shaking && clock_micros - last_shake > SHAKE_DEBOUNCE) {
    PRINTF("Shake end\n");
    is_shaking = false;
  } else if (!is_shaking) {
    PRINTF("Shake start\n");
    is_shaking = true;
  }
}

void waitForNextSample() {
  // Underflow is fine here as we only take the difference,
  // which is almost guaranteed to not overflow (70mins for a frame is crazy).
  uint32_t elapsed = micros() - clock_micros;
  if (SAMPLE_MICROS > elapsed) {
    uint32_t remaining_milis = (SAMPLE_MICROS - elapsed) / 1000;
    delay(remaining_milis);
  }
  clock_micros = micros();

  // PRINTF("micros: %llu\n", elapsed);
}
