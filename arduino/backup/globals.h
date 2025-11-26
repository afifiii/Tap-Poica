#ifndef GLOBALS_H
#define GLOBALS_H

#include <TinyScreen.h>
#include <SPI.h>

#define DEBUG_PRINT 1

#if DEBUG_PRINT
#include <stdio.h>
extern char sprintbuff[100];
#define SerialMonitorInterface SerialUSB
#define PRINTF(...)                                                            \
  {                                                                            \
    sprintf(sprintbuff, __VA_ARGS__);                                          \
    SerialMonitorInterface.print(sprintbuff);                                  \
  }
#else
#define PRINTF(...)
#endif

#endif