#include "DisplayManager.h"

DisplayManager::DisplayManager(TinyScreen &display, Timer &state,
                               MenuManager &menu, BLEManager &ble)
    : _ts(display), _timer(state), _menuManager(menu), _bleManager(ble),
      _lastDisplayedDay(-1), _lastDisplayedMonth(-1), _lastDisplayedYear(-1),
      _lastAMPMDisplayed(0), _lastHourDisplayed(-1), _lastMinuteDisplayed(-1),
      _lastSecondDisplayed(-1), _lastSetBrightness(100),
      _lastAmtNotificationsShown(-1), _lastMainDisplayUpdate(0),
      _mainDisplayUpdateInterval(300), rewriteTime(true), rewriteMenu(false),
      bleConnectionDisplayedState(true), notificationLine1(""), notificationLine2("") {}

void DisplayManager::begin() {
  _menuManager.RTCZ.begin();
  _menuManager.RTCZ.setTime(16, 15, 1);
  _menuManager.RTCZ.setDate(25, 7, 16);

  _ts.begin();
  _ts.setFlip(true);
  initHomeScreen();
  _timer.requestScreenOn();
}

void DisplayManager::update() {
  if (_timer.displayOn &&
      (millisOffset() > _mainDisplayUpdateInterval + _lastMainDisplayUpdate))
    updateMainDisplay();

  if (_bleManager.getRxBufferLen()) {
    uint8_t *buffer = _bleManager.getRxBuffer();
    switch (buffer[0]) {
    case 'D':
      // expect date/time string- example: D2015 03 05 11 48 42
      int y, M, d, k, m, s;
      char *next;
      y = strtol((char *)buffer + 1, &next, 10);
      M = strtol(next, &next, 10);
      d = strtol(next, &next, 10);
      k = strtol(next, &next, 10);
      m = strtol(next, &next, 10);
      s = strtol(next, &next, 10);
      _menuManager.RTCZ.setTime(k, m, s);
      _menuManager.RTCZ.setDate(d, M, y - 2000);
      _timer.requestScreenOn();
      break;
    case '1':
      memcpy(notificationLine1, buffer + 1, _bleManager.getRxBufferLen() - 1);
      notificationLine1[_bleManager.getRxBufferLen() - 1] = '\0';
      _timer.amtNotifications = 1;
      _timer.requestScreenOn();
      break;
    case '2':
      memcpy(notificationLine2, buffer + 1, _bleManager.getRxBufferLen() - 1);
      notificationLine2[_bleManager.getRxBufferLen() - 1] = '\0';
      _timer.amtNotifications = 1;
      // rewriteMenu = true;
      updateMainDisplay();
      // doVibrate = millisOffset();
      break;
    }
    _bleManager.resetRxBuffer();
  }

  // Handle button presses
  uint8_t buttonReleased = 1;
  byte buttons = _ts.getButtons();
  if (buttonReleased && buttons) {
    if (_timer.displayOn)
      buttonPress(buttons);
    _timer.requestScreenOn();
    buttonReleased = 0;
  }
  if (!buttonReleased && !(buttons & 0x0F)) {
    buttonReleased = 1;
  }

  _timer.handleSleep();
}

void DisplayManager::initHomeScreen() {
  _ts.clearWindow(0, 12, 96, 64);
  rewriteTime = true;
  rewriteMenu = true;
  updateMainDisplay();
}

void DisplayManager::updateDateDisplay() {
  // #if defined (ARDUINO_ARCH_AVR)
  //   int currentDay = day();
  //   int currentMonth = month();
  //   int currentYear = year();
  // #elif defined(ARDUINO_ARCH_SAMD)
  int currentDay = _menuManager.RTCZ.getDay();
  int currentMonth = _menuManager.RTCZ.getMonth();
  int currentYear = _menuManager.RTCZ.getYear();
  // #endif
  if ((_lastDisplayedDay == currentDay) &&
      (_lastDisplayedMonth == currentMonth) &&
      (_lastDisplayedYear == currentYear))
    return;
  _lastDisplayedDay = currentDay;
  _lastDisplayedMonth = currentMonth;
  _lastDisplayedYear = currentYear;
  _ts.setFont(FONT_10_PT);
  _ts.fontColor(DEFAULT_FONT_COLOR, DEFAULT_FONT_BG);
  _ts.setCursor(2, 2);
  // #if defined (ARDUINO_ARCH_AVR)
  //   display.print(dayShortStr(weekday()));
  //   display.print(' ');
  //   display.print(month());
  //   display.print('/');
  //   display.print(day());
  //   display.print(F("  "));
  // #elif defined(ARDUINO_ARCH_SAMD)
  const char *wkday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  time_t currentTime = _menuManager.RTCZ.getEpoch();
  struct tm *wkdaycalc = gmtime(&currentTime);
  _ts.print(wkday[wkdaycalc->tm_wday]);
  _ts.print(' ');
  _ts.print(_menuManager.RTCZ.getMonth());
  _ts.print('/');
  _ts.print(_menuManager.RTCZ.getDay());
  _ts.print(F("  "));
  // #endif
  bleConnectionDisplayedState = ~_bleManager.bleConnectionState;
  updateBLEstatusDisplay();
}

void DisplayManager::updateMainDisplay() {
  if (_lastSetBrightness != _timer.brightness) {
    _ts.setBrightness(_timer.brightness);
    _lastSetBrightness = _timer.brightness;
  }
  updateDateDisplay();
  updateBLEstatusDisplay();
  displayBattery();
  if (_timer.currentDisplayState == Timer::STATE_HOME) {
    updateTimeDisplay();
    if (rewriteMenu ||
        _lastAmtNotificationsShown != _timer.amtNotifications) {
      _lastAmtNotificationsShown = _timer.amtNotifications;
      _ts.setFont(FONT_10_PT);
      _ts.clearWindow(0, _menuManager.menuTextY[2], 96, 13);
      if (_timer.amtNotifications) {
        int printPos = 48 - (_ts.getPrintWidth(notificationLine1) / 2);
        if (printPos < 0)
          printPos = 0;
        _ts.setCursor(printPos, _menuManager.menuTextY[2]);
        _ts.print(notificationLine1);
      }
      _ts.setCursor(0, _menuManager.menuTextY[3]);
      _ts.print(F("< Menu          "));
      char viewStr[] = "View >";
      int Xpos = 95 - _ts.getPrintWidth(viewStr);
      _ts.setCursor(Xpos, _menuManager.menuTextY[3]);
      _ts.print(viewStr);
      rewriteMenu = false;
    }
  }
  _lastMainDisplayUpdate = millisOffset();
}

void DisplayManager::updateTimeDisplay() {
  int currentHour, currentMinute, currentSecond;
  // #if defined (ARDUINO_ARCH_AVR)
  //   currentHour = hour();
  //   currentMinute = minute();
  //   currentSecond = second();
  // #elif defined(ARDUINO_ARCH_SAMD)
  currentHour = _menuManager.RTCZ.getHours();
  currentMinute = _menuManager.RTCZ.getMinutes();
  currentSecond = _menuManager.RTCZ.getSeconds();
  // #endif
  if (_timer.currentDisplayState != Timer::STATE_HOME)
    return;
  char displayX;
  int hour12 = currentHour;
  int AMPM = 1;
  if (hour12 > 12) {
    AMPM = 2;
    hour12 -= 12;
  }
  _ts.fontColor(DEFAULT_FONT_COLOR, DEFAULT_FONT_BG);
  if (rewriteTime || _lastHourDisplayed != hour12) {
    _ts.setFont(FONT_22_PT);
    _lastHourDisplayed = hour12;
    displayX = 0;
    _ts.setCursor(displayX, TIME_Y);
    if (_lastHourDisplayed < 10)
      _ts.print('0');
    _ts.print(_lastHourDisplayed);
    _ts.write(':');
    if (_lastAMPMDisplayed != AMPM) {
      if (AMPM == 2)
        _ts.fontColor(INACTIVE_FONT_COLOR, INACTIVE_FONT_BG);
      _ts.setFont(FONT_10_PT);
      _ts.setCursor(displayX + 80, TIME_Y - 0);
      _ts.print(F("AM"));
      if (AMPM == 2) {
        _ts.fontColor(DEFAULT_FONT_COLOR, DEFAULT_FONT_BG);
      } else {
        _ts.fontColor(INACTIVE_FONT_COLOR, INACTIVE_FONT_BG);
      }
      _ts.setCursor(displayX + 80, TIME_Y + 11);
      _ts.print(F("PM"));
      _ts.fontColor(DEFAULT_FONT_COLOR, DEFAULT_FONT_BG);
    }
  }

  if (rewriteTime || _lastMinuteDisplayed != currentMinute) {
    _ts.setFont(FONT_22_PT);
    _lastMinuteDisplayed = currentMinute;
    displayX = 14 + 14 - 1;
    _ts.setCursor(displayX, TIME_Y);
    if (_lastMinuteDisplayed < 10)
      _ts.print('0');
    _ts.print(_lastMinuteDisplayed);
    _ts.write(':');
  }

  if (rewriteTime || _lastSecondDisplayed != currentSecond) {
    _ts.setFont(FONT_22_PT);
    _lastSecondDisplayed = currentSecond;
    displayX = 14 + 14 + 14 + 14 - 2;
    _ts.setCursor(displayX, TIME_Y);
    if (_lastSecondDisplayed < 10)
      _ts.print('0');
    _ts.print(_lastSecondDisplayed);
  }
  rewriteTime = false;
}

void DisplayManager::updateBLEstatusDisplay() {
  if (_bleManager.bleConnectionState == bleConnectionDisplayedState)
    return;
  bleConnectionDisplayedState = _bleManager.bleConnectionState;
  int x = 62;
  int y = 6;
  int s = 2;
  uint8_t color = 0x03;
  if (_bleManager.bleConnectionState)
    color = 0xE0;
  _ts.drawLine(x, y + s + s, x, y - s - s, color);
  _ts.drawLine(x - s, y + s, x + s, y - s, color);
  _ts.drawLine(x + s, y + s, x - s, y - s, color);
  _ts.drawLine(x, y + s + s, x + s, y + s, color);
  _ts.drawLine(x, y - s - s, x + s, y - s, color);
}

void DisplayManager::displayBattery() {
  int result = 0;
  // #if defined(ARDUINO_ARCH_SAMD)
  // http://atmel.force.com/support/articles/en_US/FAQ/ADC-example
  SYSCTRL->VREF.reg |= SYSCTRL_VREF_BGOUTEN;
  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ;
  ADC->SAMPCTRL.bit.SAMPLEN = 0x1;
  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ;
  ADC->INPUTCTRL.bit.MUXPOS = 0x19; // Internal bandgap input
  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ;
  ADC->CTRLA.bit.ENABLE = 0x01; // Enable ADC
  // Start conversion
  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ;
  ADC->SWTRIG.bit.START = 1;
  // Clear the Data Ready flag
  ADC->INTFLAG.bit.RESRDY = 1;
  // Start conversion again, since The first conversion after the reference is
  // changed must not be used.
  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ;
  ADC->SWTRIG.bit.START = 1;
  // Store the value
  while (ADC->INTFLAG.bit.RESRDY == 0)
    ; // Waiting for conversion to complete
  uint32_t valueRead = ADC->RESULT.reg;
  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ;
  ADC->CTRLA.bit.ENABLE = 0x00; // Disable ADC
  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ;
  SYSCTRL->VREF.reg &= ~SYSCTRL_VREF_BGOUTEN;
  result = (((1100L * 1024L) / valueRead) + 5L) / 10L;
  uint8_t x = 70;
  uint8_t y = 3;
  uint8_t height = 5;
  uint8_t length = 20;
  uint8_t red, green;
  if (result > 325) {
    red = 0;
    green = 63;
  } else {
    red = 63;
    green = 0;
  }
  _ts.drawLine(x - 1, y, x - 1, y + height, 0xFF);     // left boarder
  _ts.drawLine(x - 1, y - 1, x + length, y - 1, 0xFF); // top border
  _ts.drawLine(x - 1, y + height + 1, x + length, y + height + 1,
              0xFF); // bottom border
  _ts.drawLine(x + length, y - 1, x + length, y + height + 1,
              0xFF); // right border
  _ts.drawLine(x + length + 1, y + 2, x + length + 1, y + height - 2,
              0xFF); // right border
  for (uint8_t i = 0; i < length; i++) {
    _ts.drawLine(x + i, y, x + i, y + height, red, green, 0);
  }
  // #endif
}

void DisplayManager::viewNotifications(uint8_t button) {
  if (button == CLEAR_BUTTON) {
    _timer.currentDisplayState = Timer::STATE_HOME;
    initHomeScreen();
  }

  if (button == SELECT_BUTTON) {
    _timer.amtNotifications = 0;
    _timer.currentDisplayState = Timer::STATE_HOME;
    initHomeScreen();
  }

  if (button)
    return;

  PRINTF("viewNotificationsInit");
  _timer.currentDisplayState = Timer::STATE_MENU;
  _ts.clearWindow(0, 12, 96, 64);
  _ts.setFont(FONT_10_PT);
  _ts.fontColor(DEFAULT_FONT_COLOR, DEFAULT_FONT_BG);
  if (_timer.amtNotifications) {
    PRINTF("amtNotifications=true");
    // _ts.setCursor(0, menuTextY[1]);
    // _ts.setCursor(0, 0);
    // _ts.print(ANCSNotificationTitle());

    int line = 0;
    int totalMessageChars = strlen(notificationLine2);
    int printedChars = 0;
    while (printedChars < totalMessageChars && line < 3) {
      char tempPrintBuff[40] = "";
      int tempPrintBuffPos = 0;
      while (_ts.getPrintWidth(tempPrintBuff) < 90 &&
             printedChars < totalMessageChars) {
        if (!(tempPrintBuffPos == 0 &&
              notificationLine2[printedChars] == ' ')) {
          tempPrintBuff[tempPrintBuffPos] = notificationLine2[printedChars];
          tempPrintBuffPos++;
        }
        printedChars++;
        tempPrintBuff[tempPrintBuffPos] = '\0';
      }
      _ts.setCursor(0, _menuManager.menuTextY[line]);
      _ts.print((char *)tempPrintBuff);
      line++;
    }

    _ts.setCursor(0, _menuManager.menuTextY[3]);
    _ts.print(F("< "));
    _ts.print("Clear");

    char backStr[] = "Back >";
    int Xpos = 95 - _ts.getPrintWidth(backStr);
    _ts.setCursor(Xpos, _menuManager.menuTextY[3]);
    _ts.print(backStr);
  } else {
    PRINTF("amtNotifications=false");
    _ts.setCursor(0, _menuManager.menuTextY[0]);
    _ts.print(F("  No notifications."));
    char backStr[] = "Back >";
    int Xpos = 95 - _ts.getPrintWidth(backStr);
    _ts.setCursor(Xpos, _menuManager.menuTextY[3]);
    _ts.print(backStr);
  }
}

void DisplayManager::buttonPress(uint8_t buttons) {
  switch (_timer.currentDisplayState) {
  case Timer::STATE_HOME:
    if (buttons == VIEW_BUTTON) {
      viewNotifications(0);
    } else if (buttons == MENU_BUTTON) {
      _menuManager.viewMenu(0);
    }
    break;
  case Timer::STATE_MENU:
    _menuManager.viewMenu(buttons);
    break;
  case Timer::STATE_EDITOR:
    int val = 0;
    _menuManager.editInt(buttons, &val, 0, NULL);
    break;
  }
}
