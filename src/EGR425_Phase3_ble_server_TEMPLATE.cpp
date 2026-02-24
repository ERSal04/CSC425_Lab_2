// // https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/
// // https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/examples/BLE_notify/BLE_notify.ino
// // https://www.electronicshub.org/esp32-ble-tutorial/
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLE2902.h>
// #include <M5Unified.h>


// ///////////////////////////////////////////////////////////////
// // Variables
// ///////////////////////////////////////////////////////////////

// BLEServer *bleServer;
// BLEService *bleService;
// BLECharacteristic *bLeCharacteristic;
// bool deviceConnected = false;
// bool previouslyConnected = false;
// int timer = 0;

// // See the following for generating UUIDs: https://www.uuidgenerator.net/
// #define SERVICE_UUID        "4d92ed41-94fc-43a2-a9e6-e17e7f804d02"
// #define CHARACTERISTIC_UUID "99f63e2d-8c68-4206-b763-da326c24009a"

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
    
// }

// ///////////////////////////////////////////////////////////////
// // Put your main code here, to run repeatedly
// ///////////////////////////////////////////////////////////////
// void loop()
// {
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

//     delay(1000);
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