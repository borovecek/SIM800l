#include "SimpleSIM800L.h"

SimpleSIM800L::SimpleSIM800L(HardwareSerial& serial, Stream& debug)
  : _serial(serial), _debug(debug) {}

void SimpleSIM800L::begin(int baud, int8_t rxPin, int8_t txPin) {
  _serial.begin(baud, SERIAL_8N1, rxPin, txPin);
}

void SimpleSIM800L::setSimPin(const char* pin) {
  _simPin = pin;
}

void SimpleSIM800L::setStartupDelay(unsigned long ms) {
  _startupDelayMs = ms;
}

void SimpleSIM800L::setCommandTimeout(unsigned long ms) {
  _cmdTimeoutMs = ms;
}

void SimpleSIM800L::setNetworkTimeout(unsigned long ms) {
  _networkTimeoutMs = ms;
}

void SimpleSIM800L::setNetworkPollInterval(unsigned long ms) {
  _networkPollIntervalMs = ms;
}

bool SimpleSIM800L::init() {
  _error = false;
  _ready = false;
  _lastError = "";

  _debug.println();
  _debug.println(F("SIM800L: start"));
  delay(_startupDelayMs);

  if (!sendATExpectOK("AT", _cmdTimeoutMs)) {
    fail("SIM800L neodpovida na AT");
    return false;
  }

  if (!sendATExpectOK("ATE0", _cmdTimeoutMs)) {
    fail("Nepodarilo se vypnout echo");
    return false;
  }

  if (!sendATExpectOK("AT+CMEE=2", _cmdTimeoutMs)) {
    fail("Nepodarilo se zapnout podrobne chyby");
    return false;
  }

  return true;
}

bool SimpleSIM800L::unlockSIMIfNeeded() {
  String response = sendATGetResponse("AT+CPIN?", _cmdTimeoutMs);

  _debug.println(F("SIM800L: stav SIM"));
  _debug.println(response);

  if (response.indexOf("+CPIN: READY") >= 0) {
    _debug.println(F("SIM800L: SIM je odemcena"));
    return true;
  }

  if (response.indexOf("+CPIN: SIM PIN") >= 0) {
    if (_simPin == nullptr || strlen(_simPin) == 0) {
      fail("SIM vyzaduje PIN, ale zadny PIN neni nastaven");
      return false;
    }

    _debug.println(F("SIM800L: SIM chce PIN, zkousim jednou zadat"));

    String cmd = String("AT+CPIN=\"") + _simPin + "\"";
    String pinResp = sendATGetResponse(cmd.c_str(), 8000);

    _debug.println(F("SIM800L: odpoved na PIN"));
    _debug.println(pinResp);

    if (pinResp.indexOf("OK") < 0) {
      fail("PIN nebyl prijat");
      return false;
    }

    delay(3000);

    String verifyResp = sendATGetResponse("AT+CPIN?", _cmdTimeoutMs);

    _debug.println(F("SIM800L: kontrola po PINu"));
    _debug.println(verifyResp);

    if (verifyResp.indexOf("+CPIN: READY") >= 0) {
      _debug.println(F("SIM800L: PIN prijat"));
      return true;
    }

    fail("PIN byl odeslan, ale SIM neni READY");
    return false;
  }

  if (response.indexOf("+CPIN: SIM PUK") >= 0) {
    fail("SIM je zablokovana a chce PUK");
    return false;
  }

  fail("Neznamy stav SIM");
  return false;
}

bool SimpleSIM800L::waitForNetwork() {
  _debug.println(F("SIM800L: cekam na sit"));

  unsigned long start = millis();

  while (millis() - start < _networkTimeoutMs) {
    String creg = sendATGetResponse("AT+CREG?", _cmdTimeoutMs);
    _debug.println(F("SIM800L: CREG"));
    _debug.println(creg);

    if (isRegistered(creg)) {
      _ready = true;
      _debug.println(F("SIM800L: registrovano pres CREG"));
      return true;
    }

    String cgreg = sendATGetResponse("AT+CGREG?", _cmdTimeoutMs);
    _debug.println(F("SIM800L: CGREG"));
    _debug.println(cgreg);

    if (isRegistered(cgreg)) {
      _ready = true;
      _debug.println(F("SIM800L: registrovano pres CGREG"));
      return true;
    }

    delay(_networkPollIntervalMs);
  }

  fail("Nepodarilo se zaregistrovat do site");
  return false;
}

bool SimpleSIM800L::isReady() const {
  return _ready;
}

bool SimpleSIM800L::hasError() const {
  return _error;
}

String SimpleSIM800L::getLastError() const {
  return _lastError;
}

String SimpleSIM800L::getSignalQuality() {
  return sendATGetResponse("AT+CSQ", _cmdTimeoutMs);
}

String SimpleSIM800L::getOperator() {
  return sendATGetResponse("AT+COPS?", _cmdTimeoutMs);
}

String SimpleSIM800L::getSimStatus() {
  return sendATGetResponse("AT+CPIN?", _cmdTimeoutMs);
}

void SimpleSIM800L::fail(const String& msg) {
  _error = true;
  _ready = false;
  _lastError = msg;
  _debug.print(F("SIM800L CHYBA: "));
  _debug.println(msg);
}

void SimpleSIM800L::clearInput() {
  while (_serial.available()) {
    _serial.read();
  }
}

String SimpleSIM800L::sendATGetResponse(const char* cmd, unsigned long timeoutMs) {
  clearInput();
  _serial.println(cmd);

  String response;
  unsigned long start = millis();

  while (millis() - start < timeoutMs) {
    while (_serial.available()) {
      char c = _serial.read();
      response += c;
    }

    if (response.indexOf("OK") >= 0 || response.indexOf("ERROR") >= 0) {
      break;
    }
  }

  response.trim();
  return response;
}

bool SimpleSIM800L::sendATExpectOK(const char* cmd, unsigned long timeoutMs) {
  String resp = sendATGetResponse(cmd, timeoutMs);
  _debug.print(F(">> "));
  _debug.println(cmd);
  _debug.println(resp);
  return resp.indexOf("OK") >= 0;
}

bool SimpleSIM800L::isRegistered(const String& resp) {
  return (resp.indexOf(",1") >= 0 || resp.indexOf(",5") >= 0);
}