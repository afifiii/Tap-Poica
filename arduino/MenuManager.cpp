#include "MenuManager.h"

// Menu data
typedef struct {
  const uint8_t amtLines;
  const char *const *strings;
  void (MenuManager::*selectionHandler)(uint8_t);
} menu_info;

static const char PROGMEM mainMenuStrings0[] = "Set date/time";
static const char PROGMEM mainMenuStrings1[] = "Set auto off";
static const char PROGMEM mainMenuStrings2[] = "Set brightness";

static const char *const PROGMEM mainMenuStrings[] = {
    mainMenuStrings0,
    mainMenuStrings1,
    mainMenuStrings2,
};

const menu_info mainMenuInfo = {
    3,
    mainMenuStrings,
    &MenuManager::mainMenu,
};

static const char PROGMEM dateTimeMenuStrings0[] = "Set Year";
static const char PROGMEM dateTimeMenuStrings1[] = "Set Month";
static const char PROGMEM dateTimeMenuStrings2[] = "Set Day";
static const char PROGMEM dateTimeMenuStrings3[] = "Set Hour";
static const char PROGMEM dateTimeMenuStrings4[] = "Set Minute";
static const char PROGMEM dateTimeMenuStrings5[] = "Set Second";

static const char *const PROGMEM dateTimeMenuStrings[] = {
    dateTimeMenuStrings0, dateTimeMenuStrings1, dateTimeMenuStrings2,
    dateTimeMenuStrings3, dateTimeMenuStrings4, dateTimeMenuStrings5,
};

const menu_info dateTimeMenuInfo = {
    6,
    dateTimeMenuStrings,
    &MenuManager::dateTimeMenu,
};

const menu_info menuList[] = {mainMenuInfo, dateTimeMenuInfo};
#define mainMenuIndex 0
#define dateTimeMenuIndex 1

MenuManager::MenuManager(Timer &state, TinyScreen &ts)
    : _timer(state), _menuHistoryIndex(0), _currentMenu(0), _currentMenuLine(0),
      _lastMenuLine(-1), _currentSelectionLine(0), _lastSelectionLine(-1),
      _currentVal(0), _currentDigit(0), _maxDigit(4), _originalVal(nullptr),
      _editIntCallBack(nullptr), _dateTimeSelection(0), _dateTimeVariable(0),
      _display(ts), menuTextY({12, 25, 38, 51}) {
  for (int i = 0; i < 4; i++) {
    _digits[i] = 0;
  }
  for (int i = 0; i < 5; i++) {
    _menuHistory[i] = 0;
  }
}

void MenuManager::_newMenu(int8_t newIndex) {
  _currentMenuLine = 0;
  _lastMenuLine = -1;
  _currentSelectionLine = 0;
  _lastSelectionLine = -1;
  if (newIndex >= 0) {
    _menuHistory[_menuHistoryIndex++] = _currentMenu;
    _currentMenu = newIndex;
  } else {
    if (_timer.currentDisplayState == Timer::STATE_MENU) {
      _menuHistoryIndex--;
      _currentMenu = _menuHistory[_menuHistoryIndex];
    }
  }
  if (_menuHistoryIndex) {
    _timer.currentDisplayState = Timer::STATE_MENU;
    PRINTF("New menu index %u\n", _currentMenu);
  } else {
    _timer.currentDisplayState = Timer::STATE_HOME;
    PRINTF("New menu index home\n");
  }
}

uint8_t MenuManager::editInt(uint8_t button, int *inVal, char *intName,
                             void (MenuManager::*cb)()) {
  PRINTF("editInt");
  if (!button) {
    PRINTF("editIntInit");
    _editIntCallBack = cb;
    _timer.currentDisplayState = Timer::STATE_EDITOR;
    _currentDigit = 0;
    _originalVal = inVal;
    _currentVal = *_originalVal;
    _digits[3] = _currentVal % 10;
    _currentVal /= 10;
    _digits[2] = _currentVal % 10;
    _currentVal /= 10;
    _digits[1] = _currentVal % 10;
    _currentVal /= 10;
    _digits[0] = _currentVal % 10;
    _currentVal = *_originalVal;
    _display.clearWindow(0, 12, 96, 64);
    _display.setFont(FONT_10_PT);
    _display.fontColor(DEFAULT_FONT_COLOR, DEFAULT_FONT_BG);
    _display.setCursor(0, menuTextY[0]);
    _display.print(F("< back/undo"));
    _display.setCursor(90, menuTextY[0]);
    _display.print('^');
    _display.setCursor(10, menuTextY[1]);
    _display.print(intName);
    _display.setCursor(0, menuTextY[3]);
    _display.print(F("< next/save"));
    _display.setCursor(90, menuTextY[3]);
    _display.print('v');
  } else if (button == UP_BUTTON) {
    if (_digits[_currentDigit] < 9)
      _digits[_currentDigit]++;
  } else if (button == DOWN_BUTTON) {
    if (_digits[_currentDigit] > 0)
      _digits[_currentDigit]--;
  } else if (button == SELECT_BUTTON) {
    if (_currentDigit >= _maxDigit - 1) {
      // save
      int newValue = (_digits[3]) + (_digits[2] * 10) + (_digits[1] * 100) +
                     (_digits[0] * 1000);
      *_originalVal = newValue;
      viewMenu(BACK_BUTTON);
      if (_editIntCallBack) {
        (this->*_editIntCallBack)();
        _editIntCallBack = NULL;
      }
      return 1;
    }
    _currentDigit++;
  } else if (button == BACK_BUTTON) {
    if (_currentDigit <= 0) {
      PRINTF("back");
      viewMenu(BACK_BUTTON);
      return 0;
    }
    _currentDigit--;
  }
  _display.setCursor(10, menuTextY[2]);
  for (uint8_t i = 0; i < 4; i++) {
    if (i != _currentDigit)
      _display.fontColor(INACTIVE_FONT_COLOR, DEFAULT_FONT_BG);
    _display.print(_digits[i]);
    if (i != _currentDigit)
      _display.fontColor(DEFAULT_FONT_COLOR, DEFAULT_FONT_BG);
  }
  _display.print(F("   "));
  return 0;
}

void MenuManager::mainMenu(uint8_t selection) {
  PRINTF("mainMenuHandler");
  switch (selection) {
  case 0: {
    _newMenu(dateTimeMenuIndex);
    return;
  }
  case 1: {
    char buffer[20];
    strcpy_P(buffer, (PGM_P)pgm_read_word(
                         &(menuList[mainMenuIndex].strings[selection])));
    editInt(0, &_timer.sleepTimeout, buffer, NULL);
    return;
  }
  case 2: {
    char buffer[20];
    strcpy_P(buffer, (PGM_P)pgm_read_word(
                         &(menuList[mainMenuIndex].strings[selection])));
    editInt(0, &_timer.brightness, buffer, NULL);
    return;
  }
  }
}

void MenuManager::_saveChangeCallback() {
  // #if defined(ARDUINO_ARCH_SAMD)
  int timeData[] = {RTCZ.getYear(),  RTCZ.getMonth(),   RTCZ.getDay(),
                    RTCZ.getHours(), RTCZ.getMinutes(), RTCZ.getSeconds()};
  timeData[_dateTimeSelection] = _dateTimeVariable;
  RTCZ.setTime(timeData[3], timeData[4], timeData[5]);
  RTCZ.setDate(timeData[2], timeData[1], timeData[0] - 2000);
  // #endif
  PRINTF("set time %d\n", _dateTimeVariable);
}

void MenuManager::dateTimeMenu(uint8_t selection) {
  if (selection < 0 || selection >= 6)
    return;
  // #if defined(ARDUINO_ARCH_SAMD)
  int timeData[] = {RTCZ.getYear(),  RTCZ.getMonth(),   RTCZ.getDay(),
                    RTCZ.getHours(), RTCZ.getMinutes(), RTCZ.getSeconds()};
  // #endif
  _dateTimeVariable = timeData[selection];
  _dateTimeSelection = selection;
  char buffer[20];
  strcpy_P(buffer, (PGM_P)pgm_read_word(
                       &(menuList[dateTimeMenuIndex].strings[selection])));
  editInt(0, &_dateTimeVariable, buffer, &MenuManager::_saveChangeCallback);
}

void MenuManager::viewMenu(uint8_t button) {
  if (!button) {
    _newMenu(mainMenuIndex);
    _display.clearWindow(0, 12, 96, 64);
  } else {
    if (button == UP_BUTTON) {
      if (_currentSelectionLine > 0) {
        _currentSelectionLine--;
      } else if (_currentMenuLine > 0) {
        _currentMenuLine--;
      }
    } else if (button == DOWN_BUTTON) {
      if (_currentSelectionLine < menuList[_currentMenu].amtLines - 1 &&
          _currentSelectionLine < 3) {
        _currentSelectionLine++;
      } else if (_currentSelectionLine + _currentMenuLine <
                 menuList[_currentMenu].amtLines - 1) {
        _currentMenuLine++;
      }
    } else if (button == SELECT_BUTTON) {
      (this->*menuList[_currentMenu].selectionHandler)(_currentMenuLine +
                                                       _currentSelectionLine);
    } else if (button == BACK_BUTTON) {
      _newMenu(-1);
      if (!_menuHistoryIndex)
        return;
    }
  }
  _display.setFont(FONT_10_PT);
  if (_lastMenuLine == _currentMenuLine &&
      _lastSelectionLine == _currentSelectionLine)
    return;

  for (int i = 0; i < 4; i++) {
    _display.setCursor(7, menuTextY[i]);
    if (i == _currentSelectionLine) {
      _display.fontColor(DEFAULT_FONT_COLOR, INACTIVE_FONT_BG);
    } else {
      _display.fontColor(INACTIVE_FONT_COLOR, INACTIVE_FONT_BG);
    }
    if (_currentMenuLine + i < menuList[_currentMenu].amtLines) {
      char buffer[20];
      strcpy_P(buffer,
               (PGM_P)pgm_read_word(
                   &(menuList[_currentMenu].strings[_currentMenuLine + i])));
      _display.print(buffer);
    }
    for (uint8_t i = 0; i < 25; i++)
      _display.write(' ');
    if (i == 0) {
      _display.fontColor(DEFAULT_FONT_COLOR, INACTIVE_FONT_BG);
      _display.setCursor(0, menuTextY[0]);
      _display.print('<');
      _display.setCursor(90, menuTextY[0]);
      _display.print('^');
    }
    if (i == 3) {
      _display.fontColor(DEFAULT_FONT_COLOR, INACTIVE_FONT_BG);
      _display.setCursor(0, menuTextY[3]);
      _display.print('>');
      _display.setCursor(90, menuTextY[3]);
      _display.print('v');
    }
  }
  _lastMenuLine = _currentMenuLine;
  _lastSelectionLine = _currentSelectionLine;
}
