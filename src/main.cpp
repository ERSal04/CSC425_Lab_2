#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLE2902.h>
#include <M5Core2.h>
#include <Adafruit_seesaw.h>
#include <math.h>

///////////////////////////////////////////////////////////////
// Shared BLE Protocol (must match server)
///////////////////////////////////////////////////////////////
static BLEUUID SERVICE_UUID("4d92ed41-94fc-43a2-a9e6-e17e7f804d02");
static BLEUUID SERVER_POSITION_UUID("99f63e2d-8c68-4206-b763-da326c24009a");
static BLEUUID CLIENT_POSITION_UUID("a1b2c3d4-e5f6-7890-abcd-ef1234567890");
static const char *SERVER_NAME = "EGR425_BLE_Tag_Server";

///////////////////////////////////////////////////////////////
// Display / Game
///////////////////////////////////////////////////////////////
static const int SCREEN_W = 320;
static const int SCREEN_H = 240;
static const int DOT_RADIUS = 6;
static const int JOY_CENTER = 512;
static const int JOY_DEADZONE = 60;
static const unsigned long DISPLAY_REFRESH_MS = 33;   // ~30 FPS
static const unsigned long POSITION_WRITE_MS = 60;    // ~16 Hz
static const unsigned long RESCAN_INTERVAL_MS = 1200; // retry cadence

int localPosX = 80;
int localPosY = 120;
int localSpeed = 1;
int remotePosX = 240;
int remotePosY = 120;
bool hasRemotePosition = false;

bool gameOver = false;
unsigned long gameStartMs = 0;
float gameTimeSeconds = 0.0f;

unsigned long lastDrawMs = 0;
unsigned long lastWriteMs = 0;
int lastSentX = -1;
int lastSentY = -1;

///////////////////////////////////////////////////////////////
// Controller
///////////////////////////////////////////////////////////////
Adafruit_seesaw controller;
int lastJoyX = 512;
int lastJoyY = 512;

#define BUTTON_X 6
#define BUTTON_Y 2
#define BUTTON_A 5
#define BUTTON_B 1
#define BUTTON_SELECT 0
#define BUTTON_START 16
uint32_t button_mask = (1UL << BUTTON_X) | (1UL << BUTTON_Y) | (1UL << BUTTON_START) |
                       (1UL << BUTTON_A) | (1UL << BUTTON_B) | (1UL << BUTTON_SELECT);

///////////////////////////////////////////////////////////////
// BLE Client State
///////////////////////////////////////////////////////////////
BLEClient *bleClient = nullptr;
BLERemoteCharacteristic *remoteServerPositionChar = nullptr;
BLERemoteCharacteristic *remoteClientWriteChar = nullptr;
BLEAdvertisedDevice *discoveredServer = nullptr;

bool shouldConnect = false;
bool shouldScan = true;
bool isConnected = false;
unsigned long lastScanAttemptMs = 0;
String statusLine = "Booting...";

///////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////
void drawMainScreen();
void drawStatusScreen(const String &line1, const String &line2 = "");
void drawGameOverScreen(float seconds);
void updateLocalPosition(int joyX, int joyY, uint32_t buttons);
void sendClientPosition(bool force = false);
bool parsePositionString(const String &text, int &xOut, int &yOut);
bool connectToServer();
void startScanIfNeeded();
void resetClientState(const char *reason);
bool checkCollision();

///////////////////////////////////////////////////////////////
// BLE helpers
///////////////////////////////////////////////////////////////
class ClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient *client) override {
    (void)client;
    Serial.println("[CLIENT] Connected callback fired.");
  }

  void onDisconnect(BLEClient *client) override {
    (void)client;
    Serial.println("[CLIENT] Disconnected callback fired.");
    resetClientState("Disconnected. Rescanning...");
  }
};

static void notifyCallback(BLERemoteCharacteristic *pChar, uint8_t *pData, size_t length, bool isNotify) {
  (void)pChar;
  (void)isNotify;

  if (length == 0 || pData == nullptr) {
    return;
  }

  std::string payload((char *)pData, length);
  int parsedX = remotePosX;
  int parsedY = remotePosY;

  if (parsePositionString(String(payload.c_str()), parsedX, parsedY)) {
    remotePosX = parsedX;
    remotePosY = parsedY;
    hasRemotePosition = true;
    Serial.printf("[CLIENT] Notify remote position = (%d, %d)\n", remotePosX, remotePosY);

    if (!gameOver && checkCollision()) {
      gameTimeSeconds = (millis() - gameStartMs) / 1000.0f;
      gameOver = true;
      drawGameOverScreen(gameTimeSeconds);
    }
  } else {
    Serial.printf("[CLIENT] Invalid notify payload: '%s'\n", payload.c_str());
  }
}

class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    bool hasService = advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(SERVICE_UUID);
    if (!hasService) {
      return;
    }

    Serial.printf("[CLIENT] Found server candidate: name='%s' addr=%s\n",
                  advertisedDevice.getName().c_str(),
                  advertisedDevice.getAddress().toString().c_str());

    if (discoveredServer != nullptr) {
      delete discoveredServer;
      discoveredServer = nullptr;
    }

    discoveredServer = new BLEAdvertisedDevice(advertisedDevice);
    shouldConnect = true;
    shouldScan = false;
    BLEDevice::getScan()->stop();
  }
};

bool connectToServer() {
  if (discoveredServer == nullptr) {
    Serial.println("[CLIENT] No discovered server to connect to.");
    return false;
  }

  if (bleClient == nullptr) {
    bleClient = BLEDevice::createClient();
    bleClient->setClientCallbacks(new ClientCallbacks());
  }

  drawStatusScreen("Connecting to server...", discoveredServer->getAddress().toString().c_str());
  Serial.printf("[CLIENT] Connecting to %s (%s)\n",
                discoveredServer->getName().c_str(),
                discoveredServer->getAddress().toString().c_str());

  if (!bleClient->connect(discoveredServer)) {
    Serial.println("[CLIENT] Connection failed.");
    return false;
  }

  BLERemoteService *service = bleClient->getService(SERVICE_UUID);
  if (service == nullptr) {
    Serial.println("[CLIENT] Service UUID not found on server.");
    bleClient->disconnect();
    return false;
  }

  remoteServerPositionChar = service->getCharacteristic(SERVER_POSITION_UUID);
  remoteClientWriteChar = service->getCharacteristic(CLIENT_POSITION_UUID);

  if (remoteServerPositionChar == nullptr || remoteClientWriteChar == nullptr) {
    Serial.println("[CLIENT] Missing required characteristic(s).");
    bleClient->disconnect();
    return false;
  }

  if (!remoteClientWriteChar->canWrite()) {
    Serial.println("[CLIENT] Client write characteristic is not writable.");
    bleClient->disconnect();
    return false;
  }

  if (remoteServerPositionChar->canRead()) {
    std::string value = remoteServerPositionChar->readValue();
    int parsedX = remotePosX;
    int parsedY = remotePosY;
    if (parsePositionString(String(value.c_str()), parsedX, parsedY)) {
      remotePosX = parsedX;
      remotePosY = parsedY;
      hasRemotePosition = true;
    }
    Serial.printf("[CLIENT] Initial server position = %s\n", value.c_str());
  }

  if (remoteServerPositionChar->canNotify()) {
    remoteServerPositionChar->registerForNotify(notifyCallback);
    Serial.println("[CLIENT] Registered for server notifications.");
  }

  isConnected = true;
  shouldScan = false;
  gameOver = false;
  gameStartMs = millis();
  statusLine = "Connected";

  sendClientPosition(true);
  drawMainScreen();
  return true;
}

void resetClientState(const char *reason) {
  isConnected = false;
  shouldConnect = false;
  shouldScan = true;
  hasRemotePosition = false;
  remoteServerPositionChar = nullptr;
  remoteClientWriteChar = nullptr;
  lastSentX = -1;
  lastSentY = -1;
  statusLine = String(reason);
  drawStatusScreen(reason, "Searching for server...");

  if (bleClient != nullptr && bleClient->isConnected()) {
    bleClient->disconnect();
  }
}

void startScanIfNeeded() {
  unsigned long now = millis();
  if (!shouldScan || shouldConnect || isConnected) {
    return;
  }
  if (now - lastScanAttemptMs < RESCAN_INTERVAL_MS) {
    return;
  }

  lastScanAttemptMs = now;
  Serial.println("[CLIENT] Starting BLE scan...");
  drawStatusScreen("Scanning for server...", SERVER_NAME);
  BLEDevice::getScan()->start(3, false);
}

///////////////////////////////////////////////////////////////
// Arduino setup / loop
///////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  delay(100);

  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  Serial.println("[CLIENT] Booting client...");
  drawStatusScreen("Starting client...", "Init BLE + joystick");

  if (!controller.begin(0x50)) {
    Serial.println("[CLIENT] Could not find Gamepad QT controller.");
    drawStatusScreen("Controller not found", "Check wiring / power");
    while (true) {
      delay(100);
    }
  }

  controller.pinModeBulk(button_mask, INPUT_PULLUP);
  controller.setGPIOInterrupts(button_mask, 1);
  randomSeed(micros());

  BLEDevice::init("EGR425_BLE_Tag_Client");
  BLEScan *scan = BLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  scan->setInterval(1349);
  scan->setWindow(449);
  scan->setActiveScan(true);

  shouldScan = true;
  shouldConnect = false;
  lastScanAttemptMs = 0;
  drawStatusScreen("Client ready.", "Scanning...");
}

void loop() {
  M5.update();

  if (gameOver) {
    delay(40);
    return;
  }

  if (shouldConnect) {
    shouldConnect = false;
    if (!connectToServer()) {
      Serial.println("[CLIENT] Connect attempt failed. Will rescan.");
      resetClientState("Connect failed");
    }
  }

  if (!isConnected) {
    startScanIfNeeded();
    delay(20);
    return;
  }

  int joyX = 1023 - controller.analogRead(14);
  int joyY = 1023 - controller.analogRead(15);
  lastJoyX = joyX;
  lastJoyY = joyY;

  uint32_t buttons = controller.digitalReadBulk(button_mask);
  updateLocalPosition(joyX, joyY, buttons);
  sendClientPosition(false);

  if (hasRemotePosition && checkCollision()) {
    gameTimeSeconds = (millis() - gameStartMs) / 1000.0f;
    gameOver = true;
    drawGameOverScreen(gameTimeSeconds);
    return;
  }

  if (millis() - lastDrawMs >= DISPLAY_REFRESH_MS) {
    lastDrawMs = millis();
    drawMainScreen();
  }

  if (bleClient != nullptr && !bleClient->isConnected()) {
    Serial.println("[CLIENT] Detected dropped connection in loop.");
    resetClientState("Link lost");
  }

  delay(10);
}

///////////////////////////////////////////////////////////////
// Helpers
///////////////////////////////////////////////////////////////
bool parsePositionString(const String &text, int &xOut, int &yOut) {
  String s = text;
  s.trim();
  int dash = s.indexOf('-');
  if (dash <= 0 || dash >= s.length() - 1) {
    return false;
  }

  String xs = s.substring(0, dash);
  String ys = s.substring(dash + 1);
  xOut = constrain(xs.toInt(), DOT_RADIUS, SCREEN_W - DOT_RADIUS);
  yOut = constrain(ys.toInt(), DOT_RADIUS, SCREEN_H - DOT_RADIUS);
  return true;
}

void sendClientPosition(bool force) {
  if (!isConnected || remoteClientWriteChar == nullptr) {
    return;
  }

  unsigned long now = millis();
  bool moved = (localPosX != lastSentX || localPosY != lastSentY);
  if (!force && (!moved || (now - lastWriteMs < POSITION_WRITE_MS))) {
    return;
  }

  String payload = String(localPosX) + "-" + String(localPosY);
  std::string value(payload.c_str());
  remoteClientWriteChar->writeValue(value, false);
  lastWriteMs = now;
  lastSentX = localPosX;
  lastSentY = localPosY;
  Serial.printf("[CLIENT] Wrote local position = %s\n", payload.c_str());
}

void updateLocalPosition(int joyX, int joyY, uint32_t buttons) {
  int dx = joyX - JOY_CENTER;
  int dy = joyY - JOY_CENTER;

  if (abs(dx) > JOY_DEADZONE) {
    localPosX += (dx > 0) ? localSpeed : -localSpeed;
  }
  if (abs(dy) > JOY_DEADZONE) {
    localPosY += (dy < 0) ? localSpeed : -localSpeed;
  }

  localPosX = constrain(localPosX, DOT_RADIUS, SCREEN_W - DOT_RADIUS);
  localPosY = constrain(localPosY, DOT_RADIUS, SCREEN_H - DOT_RADIUS);

  static bool startWasPressed = false;
  bool startPressed = !(buttons & (1UL << BUTTON_START));
  if (startPressed && !startWasPressed) {
    localSpeed = (localSpeed >= 5) ? 1 : localSpeed + 1;
    Serial.printf("[CLIENT] Speed changed to %d\n", localSpeed);
  }
  startWasPressed = startPressed;

  static bool selectWasPressed = false;
  bool selectPressed = !(buttons & (1UL << BUTTON_SELECT));
  if (selectPressed && !selectWasPressed) {
    localPosX = random(DOT_RADIUS, SCREEN_W - DOT_RADIUS);
    localPosY = random(DOT_RADIUS, SCREEN_H - DOT_RADIUS);
    Serial.printf("[CLIENT] Warp to (%d, %d)\n", localPosX, localPosY);
  }
  selectWasPressed = selectPressed;
}

bool checkCollision() {
  float dx = float(localPosX - remotePosX);
  float dy = float(localPosY - remotePosY);
  float dist = sqrtf(dx * dx + dy * dy);
  return dist < (DOT_RADIUS * 2);
}

void drawStatusScreen(const String &line1, const String &line2) {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.println(line1);
  if (line2.length() > 0) {
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.println(line2);
  }
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 220);
  M5.Lcd.println("BLUE = you  |  RED = other player");
  M5.Lcd.setTextSize(2);
}

void drawMainScreen() {
  M5.Lcd.fillScreen(TFT_BLACK);

  // status indicator
  M5.Lcd.fillCircle(308, 12, 6, isConnected ? TFT_GREEN : TFT_RED);

  // labels
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setCursor(6, 6);
  M5.Lcd.printf("spd:%d", localSpeed);
  if (!hasRemotePosition) {
    M5.Lcd.setCursor(6, 18);
    M5.Lcd.print("Waiting for remote...");
  }

  // players
  M5.Lcd.fillCircle(localPosX, localPosY, DOT_RADIUS, TFT_BLUE);
  if (hasRemotePosition) {
    M5.Lcd.fillCircle(remotePosX, remotePosY, DOT_RADIUS, TFT_RED);
  }

  M5.Lcd.setTextSize(2);
}

void drawGameOverScreen(float seconds) {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(50, 70);
  M5.Lcd.println("GAME OVER");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(62, 120);
  M5.Lcd.printf("Time: %.2f s", seconds);
  M5.Lcd.setCursor(24, 160);
  M5.Lcd.println("Reset both devices to play again");
  Serial.printf("[CLIENT] GAME OVER after %.2f s\n", seconds);
}
