#include <Arduino.h>
#include "SimpleSIM800L.h"

HardwareSerial simUart(2);
SimpleSIM800L modem(simUart, Serial);

const int SIM800_RX_PIN = 16;
const int SIM800_TX_PIN = 17;

void setup() {
  Serial.begin(115200);
  delay(500);

  modem.begin(9600, SIM800_RX_PIN, SIM800_TX_PIN);
  modem.setSimPin("1234");
  modem.setStartupDelay(8000);
  modem.setNetworkTimeout(30000);

  if (!modem.init()) {
    Serial.println(modem.getLastError());
    return;
  }

  if (!modem.unlockSIMIfNeeded()) {
    Serial.println(modem.getLastError());
    return;
  }

  if (!modem.waitForNetwork()) {
    Serial.println(modem.getLastError());
    return;
  }

  Serial.println("Modem pripraven.");
  Serial.println(modem.getSignalQuality());
  Serial.println(modem.getOperator());
}

void loop() {
  if (!modem.isReady()) {
    return;
  }

  // sem tvoje aplikace
}
