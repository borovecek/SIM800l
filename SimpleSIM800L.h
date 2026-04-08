#ifndef SIMPLE_SIM800L_H
#define SIMPLE_SIM800L_H

#include <Arduino.h>

class SimpleSIM800L {
public:
  SimpleSIM800L(HardwareSerial& serial, Stream& debug);

  void begin(int baud, int8_t rxPin, int8_t txPin);

  void setSimPin(const char* pin);
  void setStartupDelay(unsigned long ms);
  void setCommandTimeout(unsigned long ms);
  void setNetworkTimeout(unsigned long ms);
  void setNetworkPollInterval(unsigned long ms);

  bool init();
  bool unlockSIMIfNeeded();
  bool waitForNetwork();

  bool isReady() const;
  bool hasError() const;
  String getLastError() const;

  String getSignalQuality();
  String getOperator();
  String getSimStatus();

private:
  HardwareSerial& _serial;
  Stream& _debug;

  const char* _simPin = nullptr;

  unsigned long _startupDelayMs = 8000;
  unsigned long _cmdTimeoutMs = 5000;
  unsigned long _networkTimeoutMs = 30000;
  unsigned long _networkPollIntervalMs = 2000;

  bool _ready = false;
  bool _error = false;
  String _lastError = "";

  void fail(const String& msg);

  void clearInput();
  String sendATGetResponse(const char* cmd, unsigned long timeoutMs);
  bool sendATExpectOK(const char* cmd, unsigned long timeoutMs);

  bool isRegistered(const String& resp);
};

#endif