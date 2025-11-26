#include "BMA250.h"
#include <RTCZero.h>
#include <STBLE.h>
#include <Wire.h>
#include <time.h>

#include "UART.cpp"
#include "display.cpp"
#include "menu.cpp"

#define SerialMonitorInterface SerialUSB
#define TAP_THRESH 5600.0
#define TAP_DEBOUNCE 60000 // microseconds
#define SHAKE_THRESH 510.0
#define SHAKE_DEBOUNCE 130000 // microseconds

#define VIBRATE_PIN 6
#define VIBRATE_PIN_ACTIVE HIGH
#define VIBRATE_PIN_INACTIVE LOW

#define FLAG_IDLE 0x00
#define FLAG_TAP 0x01
#define FLAG_SHAKE_START 0x02
#define FLAG_SHAKE_END 0x04

#define BLE_DEBUG true
#define menu_debug_print true
uint32_t doVibrate = 0;

uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_can_sleep = false;
uint8_t ble_connection_state = false;
uint8_t ble_connection_displayed_state = true;

TinyScreen display = TinyScreen(TinyScreenDefault);
SampleBuffer samples;
BMA250 accel_sensor;
uint32_t clock_micros;
uint32_t last_tap = 0, last_shake = 0;
bool is_shaking = false;
uint8_t packetFlags;

uint32_t startTime = 0;
uint32_t sleepTime = 0;
unsigned long millisOffsetCount = 0;

void wakeHandler() {
  if (sleepTime) {
    millisOffsetCount += (RTCZ.getEpoch() - sleepTime);
    sleepTime = 0;
  }
}

void RTCwakeHandler() {
  // not used
}

void watchSleep() {
  if (doVibrate || ble_can_sleep)
    return;
  sleepTime = RTCZ.getEpoch();
  RTCZ.standbyMode();
}

uint8_t defaultFontColor = TS_8b_White;
uint8_t defaultFontBG = TS_8b_Black;
uint8_t inactiveFontColor = TS_8b_Gray;
uint8_t inactiveFontBG = TS_8b_Black;

uint8_t topBarHeight = 10;
uint8_t timeY = 14;
uint8_t menuTextY[4] = {12, 25, 38, 51};

unsigned long lastReceivedTime = 0;

unsigned long batteryUpdateInterval = 10000;
unsigned long lastBatteryUpdate = 0;

unsigned long sleepTimer = 0;
int sleepTimeout = 5;

uint8_t rewriteTime = true;

uint8_t displayOn = 0;
uint8_t buttonReleased = 1;
uint8_t rewriteMenu = false;
uint8_t amtNotifications = 0;
uint8_t lastAmtNotificationsShown = -1;
unsigned long mainDisplayUpdateInterval = 300;
unsigned long lastMainDisplayUpdate = 0;
char notificationLine1[20] = "";
char notificationLine2[20] = "";

uint8_t vibratePin = 6;
uint8_t vibratePinActive = HIGH;
uint8_t vibratePinInactive = LOW;

int brightness = 3;
uint8_t lastSetBrightness = 100;

const FONT_INFO &font10pt = thinPixel7_10ptFontInfo;
const FONT_INFO &font22pt = liberationSansNarrow_22ptFontInfo;

void setup() {
  RTCZ.begin();
  RTCZ.setTime(16, 15, 1); // h,m,s
  RTCZ.setDate(25, 7, 16); // d,m,y
  Wire.begin();
  SerialMonitorInterface.begin(115200);
  display.begin();
  display.setFlip(true);
  pinMode(vibratePin, OUTPUT);
  digitalWrite(vibratePin, vibratePinInactive);
  initHomeScreen();
  requestScreenOn();
  delay(100);
  BLEsetup();

  PRINTF("Initializing BMA...");
  if (accel_sensor.begin(BMA250_range_2g, BMA250_update_time_4ms)) {
    PRINTF("ERROR! NO BMA250 DETECTED!");
  }

  clock_micros = micros();
  // currentMicros = micros();
}

void loop() {
  // uint32_t elapsed = micros() - clock_micros;
  // lastDetectTime += elapsed;
  clock_micros = micros();

  // if (SAMPLE_MICROS < lastDetectTime) {
  Sample sample = accel_sensor.read();
  samples.append(sample);
  // PRINTF("last detect: %lu\n", lastDetectTime);
  if (clock_micros - last_tap > TAP_DEBOUNCE) {
    packetFlags |= detectTap();
  }
  packetFlags |= detectShake();
  uint8_t lastDetect = micros();

  uint8_t packet[5];

  packInt32(packet, clock_micros);
  packet[4] = packetFlags;
  if (packetFlags) {
    PRINTF("write took: %lu\n", micros() - lastDetect - clock_micros);
    bleWrite((char *)packet, 5);
    packetFlags = 0;
  }

  // lastDetectTime = 0;
  // }

  bleManager.update();
  displayManager.update();

  waitForNextSample();
}

uint8_t detectTap() {
  float val = samples.get(samples.len() - 1) - samples.mean();
  if (val > TAP_THRESH || val < -TAP_THRESH) {
    PRINTF("Tap\n");
    last_tap = clock_micros;
    return FLAG_TAP;
  }
  return FLAG_IDLE;
}

uint8_t detectShake() {
  float val = samples.std_dev();
  bool still_shaking = (val > SHAKE_THRESH);
  if (still_shaking) {
    last_shake = clock_micros;
  }
  if (is_shaking == still_shaking) {
    return FLAG_IDLE;
  }

  if (is_shaking && clock_micros - last_shake > SHAKE_DEBOUNCE) {
    PRINTF("Shake end\n");
    is_shaking = false;
    return FLAG_SHAKE_END;
  } else if (!is_shaking) {
    PRINTF("Shake start\n");
    is_shaking = true;
    return FLAG_SHAKE_START;
  }
  return FLAG_IDLE;
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

uint32_t unpackInt32(uint8_t *src) {
  uint32_t val = src[3];
  val <<= 8;
  val = src[2];
  val <<= 8;
  val = src[1];
  val <<= 8;
  return val | src[0];
}

void packInt32(uint8_t *d, uint32_t val) {
  d[0] = val;
  d[1] = val >> 8;
  d[2] = val >> 16;
  d[3] = val >> 24;
}
