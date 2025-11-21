#pragma once

#include <stdint.h>

typedef struct _BLEConn {
  uint16_t handle;
  uint8_t isAdvertising;
  uint8_t isConnected;
  uint8_t isBonded;
  uint8_t connectedAddr[6];
  void (*onConnect)();
  void (*onBond)();
  void (*onDisconnect)();
  // BLEChar *firstChar;
} BLEConn;

// UUID_TYPE_16
typedef struct _BLEServ {
  uint8_t UUIDType;
  uint8_t UUID[16];
  uint16_t handle;
  uint16_t handleRangeEnd;
} BLEServ;

typedef struct _BLEChar {
  uint8_t UUIDType;
  uint8_t UUID[16];
  uint16_t serviceHandle;
  uint16_t handle;
  uint16_t valueHandle;
  uint8_t properties;
  void (*onUpdate)(uint8_t *, uint8_t);
  // BLEChar *nextCharacteristic;
} BLEChar;

void uuidStrToByte(char *in, uint8_t *out, uint8_t byte_size);

class BLEManager {
public:
  BLEManager();

  bool bleConnectionState;

  void begin();
  void update();
  uint8_t *getRxBuffer();
  uint8_t getRxBufferLen();
  void resetRxBuffer();

  void advertise(char *advName, char *advUUID);
  uint8_t write(char *txData, uint8_t dataSize);

  static void setInstance(BLEManager *instance);
  static uint8_t addService(BLEServ *service, char *servUUID, uint8_t servType,
                            uint8_t maxAttrCount);
  static uint8_t addCharacteristic(BLEServ *service, BLEChar *characteristic,
                                   char *charUUID, uint8_t charValLen,
                                   uint8_t charProps, uint8_t secPermission,
                                   uint8_t gattEventMask,
                                   uint8_t encryptionKeySize);

  static void onReadRequest(uint16_t handle);
  static void onAttributeRead(uint16_t handle, uint8_t dataLen,
                              uint8_t attrDataLen, uint8_t *attrData);
  static void onAttributeModified(uint16_t handle, uint8_t dataLen,
                                  uint8_t *attrData);
  static void onAttributeNotification(uint16_t handle, uint8_t dataLength,
                                      uint16_t attrHandle, uint8_t *attrValue);
  static void onPairComplete(uint8_t status);
  static void onUnpairComplete(void);
  static void onGAPConnectionComplete(uint8_t addr[6], uint16_t handle);
  static void onGAPDisconnectionComplete(void);

private:
  BLEConn _phoneConnection;
  uint8_t _bleRxBuffer[21];
  uint8_t _bleRxBufferLen;
  volatile uint8_t _setConnectable;
  uint16_t _connectionHandle;
  int _connected;
  uint32_t _lastProcedureCompleted;
  BLEServ uartService;
  BLEChar uartTxChar;
  BLEChar uartRxChar;

  static BLEManager *instance;
  static void onConnect();
  static void onDisconnect();
  static void onBond();
};
