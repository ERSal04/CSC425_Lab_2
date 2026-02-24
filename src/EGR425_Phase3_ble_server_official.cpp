// // https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/
// // https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/examples/BLE_notify/BLE_notify.ino
// // https://www.electronicshub.org/esp32-ble-tutorial/
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLE2902.h>
// #include <M5Unified.h>
// #include <Adafruit_seesaw.h>

// ///////////////////////////////////////////////////////////////
// // Server Variables
// ///////////////////////////////////////////////////////////////

// BLEServer *bleServer;
// BLEService *bleService;
// BLECharacteristic *bLeCharacteristic;
// bool deviceConnected = false;
// bool previouslyConnected = false;
// int timer = 0;

// int last_x = 512, last_y = 512;

// // See the following for generating UUIDs: https://www.uuidgenerator.net/
// #define SERVICE_UUID        "4d92ed41-94fc-43a2-a9e6-e17e7f804d02"
// #define CHARACTERISTIC_UUID "99f63e2d-8c68-4206-b763-da326c24009a"

// ///////////////////////////////////////////////////////////////
// // Controller Variables
// ///////////////////////////////////////////////////////////////

// Adafruit_seesaw controller;

// int p1Posx = 160, p1Posy = 120;
// int p1Speed = 1;

// static const int SCREEN_W = 320;
// static const int SCREEN_H = 240;
// static const int DOT_RADIUS = 6;
// static const int JOY_CENTER = 512;
// static const int JOY_DEADZONE = 60;

// #define BUTTON_X 6
// #define BUTTON_Y 2
// #define BUTTON_A 5
// #define BUTTON_B 1
// #define BUTTON_SELECT 0
// #define BUTTON_START 16
// uint32_t button_mask = (1UL << BUTTON_X) | (1UL << BUTTON_Y) | (1UL << BUTTON_START) | 
//                        (1UL << BUTTON_A) | (1UL << BUTTON_B) | (1UL << BUTTON_SELECT);


// void showGameOverScreen();
// void updateP1Pos(int joyX, int joyY, uint32_t buttons);
// void drawScreen();
// ///////////


// // Bluetooth callback methods
// class myServerCallbacks: public BLEServerCallbacks {
//     void onConnect(BLEServer *pServer) {
//         deviceConnected = true;
//         Serial.println("Bluetooth device connected...");
//     }

//     void onDisconnect(BLEServer *pServer) {
//         deviceConnected = false;
//         Serial.println("Device disconnected...");
//     }
// };

// ///////////////////////////////////////////////////////////////
// // Forward Declarations
// ///////////////////////////////////////////////////////////////
// void broadcastBleServer();
// void drawScreenTextWithBackground(String text, int backgroundColor);

// ///////////////////////////////////////////////////////////////
// // Put your setup code here, to run once
// ///////////////////////////////////////////////////////////////
// void setup() {

//     // Init device
//     M5.begin();
//     M5.Lcd.setTextSize(2);
//     Serial.println("Starting BLE...");
    
//     // Initialize M5 as BLE server...
//     BLEDevice::init("ElijahSalgado's M5Core2...");

//     drawScreenTextWithBackground("Initializing BLE...", TFT_CYAN);
//     broadcastBleServer();
//     drawScreenTextWithBackground("Broadcasting service/characteristic as BLE server...", TFT_BLUE);

//     Serial.begin(115200);

//     while(!Serial) {
//         delay(10);
//     }

//     // Use adafruit library to connect to your device
//     if (!controller.begin(0x50)) {
//     Serial.println("Could not find Controller locally..."); 
//         while (1);
//     }
//     Serial.println("Found Controller!");

//     controller.pinModeBulk(button_mask, INPUT_PULLUP);
//     controller.setGPIOInterrupts(button_mask, 1);
//     randomSeed(micros());

//     M5.Lcd.clear();
//     M5.Lcd.fillScreen(TFT_BLACK);

//     drawScreen();
    
// }

// ///////////////////////////////////////////////////////////////
// // Put your main code here, to run repeatedly
// ///////////////////////////////////////////////////////////////
// void loop()
// {
//     // TODO: turn this into a function
//     if (deviceConnected) {
//         // //  1. Update characteristic value (Which is read by client)
//         // timer++;
//         // bLeCharacteristic->setValue(timer);
//         // Serial.printf("%d written to BLE Characteristic.\n", timer);

//         // // 2. Read the characteristic value as a string (Which is written from client)
//         std::string readValue = bLeCharacteristic->getValue();
//         Serial.printf("The new characteristic value as a string is: %s\n", readValue.c_str());
//         String valStr = readValue.c_str();
//         int val = valStr.toInt();
//         Serial.printf("The new characteristic value as an int is: %d\n", val);
//         drawScreenTextWithBackground(String(val) + " read from BLE characteristic", TFT_GREEN);        

//     } else if (previouslyConnected) {
//         drawScreenTextWithBackground("Disconnected. Reset M5 device to reinitialize BLE.", TFT_RED);
//         timer = 0;
//     }

//     // TODO: hack the mainframe (add function that reads in client side x and y position)
//     // TODO: method that draws screen accordingly

//     M5.update();
//     // delay(100); // delay in loop to slow serial output

//     // Reverse x/y values to match joystick orientation
//     int x = 1023 - controller.analogRead(14);
//     int y = 1023 - controller.analogRead(15);
    
//     if ((abs(x - last_x) > 3) || (abs(y - last_y) > 3)) {
//         Serial.print("x: "); Serial.print(x); Serial.print(", "); Serial.print("y: "); Serial.println(y);
//     }
//     last_x = x;
//     last_y = y;

//     uint32_t buttons = controller.digitalReadBulk(button_mask);

//     updateP1Pos(last_x, last_y, buttons);

//     drawScreen();


//     delay(16);
// }

// void drawScreenTextWithBackground(String text, int backgroundColor) {
//     M5.Lcd.fillScreen(backgroundColor);
//     M5.Lcd.setCursor(0,0);
//     M5.Lcd.println(text);
// }

// void broadcastBleServer() {
//     // Start broadcasting (advertising) BLE service
//     bleServer = BLEDevice::createServer();
//     bleServer->setCallbacks(new myServerCallbacks());
//     bleService = bleServer->createService(SERVICE_UUID);
//     bLeCharacteristic = bleService->createCharacteristic(
//         CHARACTERISTIC_UUID,
//         BLECharacteristic::PROPERTY_READ |
//         BLECharacteristic::PROPERTY_WRITE |
//         BLECharacteristic::PROPERTY_NOTIFY |
//         BLECharacteristic::PROPERTY_INDICATE 
//     );
//     bLeCharacteristic->setValue("Hello BLE World from Elijah");
//     bleService->start();

//     // Broadcast your bluetooth service code
//     BLEAdvertising *bleAdvertising = BLEDevice::getAdvertising();
//     bleAdvertising->addServiceUUID(SERVICE_UUID);
//     bleAdvertising->setScanResponse(true);
//     bleAdvertising->setMinPreferred(0x06); // Specifically help with iphone connection issue
//     bleAdvertising->setMinPreferred(0x12);
//     BLEDevice::startAdvertising();
//     Serial.println("Characterisic defined...you can now connect with your phone!");
// }

// void drawScreen() {
//   p1Posx = constrain(p1Posx, DOT_RADIUS, SCREEN_W - DOT_RADIUS);
//   p1Posy = constrain(p1Posy, DOT_RADIUS, SCREEN_H - DOT_RADIUS);

//   M5.Lcd.fillScreen(TFT_BLACK);
//   M5.Lcd.fillCircle(p1Posx, p1Posy, DOT_RADIUS, TFT_BLUE);
// }


// void updateP1Pos(int joyX, int joyY, uint32_t buttons) {
//   int dx = joyX - JOY_CENTER;
//   int dy = joyY - JOY_CENTER;

//   if (abs(dx) > JOY_DEADZONE) {
//     p1Posx += (dx > 0) ? p1Speed : -p1Speed;
//   }

//   if (abs(dy) > JOY_DEADZONE) {
//     p1Posy += (dy < 0) ? p1Speed : -p1Speed;
//   }

//   static bool startWasPressed = false;
//   bool startPressed = !(buttons & (1UL << BUTTON_START));
//   if (startPressed && !startWasPressed) {
//     p1Speed = (p1Speed >= 5) ? 1 : p1Speed + 1;
//     Serial.printf("p1Speed = %d\n", p1Speed);
//   }
//   startWasPressed = startPressed;

//   static bool selectWasPressed = false;
//   bool selectPressed = !(buttons & (1UL << BUTTON_SELECT));
//   if (selectPressed && !selectWasPressed) {
//     p1Posx = random(DOT_RADIUS, SCREEN_W - DOT_RADIUS);
//     p1Posy = random(DOT_RADIUS, SCREEN_H - DOT_RADIUS);
//     Serial.printf("Warp to (%d, %d)\n", p1Posx, p1Posy);
//   }
//   selectWasPressed = selectPressed;
// }
