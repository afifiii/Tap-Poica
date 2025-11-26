#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include "BLEManager.h"
#include "MenuManager.h"
#include "Timer.h"
#include <time.h>

#define TIME_Y 14

class DisplayManager {
public:
  DisplayManager(TinyScreen &display, Timer &state, MenuManager &menu,
                 BLEManager &ble);

  void begin();
  void update();

  void initHomeScreen();
  void updateMainDisplay();
  void updateTimeDisplay();
  void updateDateDisplay();
  void updateBLEstatusDisplay();
  void displayBattery();

  void viewNotifications(uint8_t button);
  void buttonPress(uint8_t buttons);

private:
  TinyScreen &_ts;
  Timer &_timer;
  MenuManager &_menuManager;
  BLEManager &_bleManager;

  uint8_t _lastDisplayedDay;
  uint8_t _lastDisplayedMonth;
  uint8_t _lastDisplayedYear;
  uint8_t _lastAMPMDisplayed;
  uint8_t _lastHourDisplayed;
  uint8_t _lastMinuteDisplayed;
  uint8_t _lastSecondDisplayed;
  uint8_t _lastSetBrightness;
  uint8_t _lastAmtNotificationsShown;
  uint32_t _lastMainDisplayUpdate;
  uint32_t _mainDisplayUpdateInterval;

  char notificationLine1[20];
  char notificationLine2[20];

  bool rewriteTime;
  bool rewriteMenu;
  bool bleConnectionDisplayedState;
};

#endif
