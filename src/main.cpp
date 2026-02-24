// Some useful resources on BLE and ESP32:
//      https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/examples/BLE_notify/BLE_notify.ino
//      https://microcontrollerslab.com/esp32-bluetooth-low-energy-ble-using-arduino-ide/
//      https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/
//      https://www.electronicshub.org/esp32-ble-tutorial/
#include <BLEDevice.h>
#include <BLE2902.h>
#include <M5Core2.h>

///////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////
static BLERemoteCharacteristic *bleRemoteCharacteristic;
static BLEAdvertisedDevice *bleRemoteServer;
static boolean doConnect = false;
static boolean doScan = false;
bool deviceConnected = false;

// See the following for generating UUIDs: https://www.uuidgenerator.net/
static BLEUUID SERVICE_UUID("4d92ed41-94fc-43a2-a9e6-e17e7f804d02"); 
static BLEUUID CHARACTERISTIC_UUID("99f63e2d-8c68-4206-b763-da326c24009a"); 

// BLE Broadcast name

static String BLE_BROADCAST_NAME = "Elijah M5Core2";

///////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////
void drawScreenTextWithBackground(String text, int backgroundColor);

///////////////////////////////////////////////////////////////
// BLE Client Callback Methods
// This method is called when the server that this client is
// connected to NOTIFIES this client (or any client listening)
// that it has changed the remote characteristic
///////////////////////////////////////////////////////////////
static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
    Serial.printf("Notify callback for characteristic %s of data length %d\n", pBLERemoteCharacteristic->getUUID().toString().c_str(), length);
    Serial.printf("\tData: %s", (char *)pData);
    std::string value = pBLERemoteCharacteristic->readValue();
    Serial.printf("\tValue was: %s", value.c_str());
}

///////////////////////////////////////////////////////////////
// BLE Server Callback Method
// These methods are called upon connection and disconnection
// to BLE service.
///////////////////////////////////////////////////////////////
class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient)
    {
        deviceConnected = true;
        Serial.println("Device connected...");
    }

    void onDisconnect(BLEClient *pclient)
    {
        deviceConnected = false;
        Serial.println("Device disconnected...");
    }
};

///////////////////////////////////////////////////////////////
// Method is called to connect to server
///////////////////////////////////////////////////////////////
bool connectToServer()
{
    // Create the client
    Serial.printf("Forming a connection to %s\n", bleRemoteServer->getName().c_str());
    BLEClient *bleClient = BLEDevice::createClient();
    bleClient->setClientCallbacks(new MyClientCallback());
    Serial.println("\tClient Connected");

    // Connect to the remove BLE Server.
    if(!bleClient->connect(bleRemoteServer)) {
        Serial.printf("Failed to connect to server (%s)\n", bleRemoteServer->getName().c_str());
    }
    Serial.printf("\tConnected tp server (%s)\n", bleRemoteServer->getName().c_str());

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *bleRemoteService = bleClient->getService(SERVICE_UUID);
    if (bleRemoteService == nullptr) {
        Serial.printf("Failed to find our service UUID: %s\n", SERVICE_UUID.toString().c_str());
        bleClient->disconnect();
        return false;
    }
    Serial.printf("\tFound our service UUID: %s\n", SERVICE_UUID.toString().c_str());

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    bleRemoteCharacteristic = bleRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
    if (bleRemoteCharacteristic == nullptr) {
        Serial.printf("Failed to find our characteristic: %s\n", CHARACTERISTIC_UUID.toString().c_str());
        bleClient->disconnect();
        return false;
    }
    Serial.printf("\tFound our charcteristic: %s\n", CHARACTERISTIC_UUID.toString().c_str());

    // Read the value of the characteristic.
    if(bleRemoteCharacteristic->canRead()) {
        std::string value = bleRemoteCharacteristic->readValue();
        Serial.printf("The characteristic value was: %s", value.c_str());
        drawScreenTextWithBackground("Initial characteristic value from server:\n\n" + String(value.c_str()), TFT_GREEN);
        delay(3000);
    }

    // Check if server's characteristic can notify client of changes and register to listen if so
    if (bleRemoteCharacteristic->canNotify()) {
        bleRemoteCharacteristic->registerForNotify(notifyCallback);
    }

    // deviceConnected = true;
    return true;
}

///////////////////////////////////////////////////////////////
// Scan for BLE servers and find the first one that advertises
// the service we are looking for.
///////////////////////////////////////////////////////////////
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    /**
     * Called for each advertising BLE server.
     */
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        // Print device found
        Serial.print("BLE Advertised Device Found");
        Serial.printf("\tName: %s\n", advertisedDevice.getName().c_str());

        // More debugging print
        Serial.printf("\tAddress: %s\n", advertisedDevice.getAddress().toString().c_str());
        Serial.printf("\tHas a ServiceUUID: %s\n", advertisedDevice.haveServiceUUID() ? "True" : "False");
        for (int i = 0; i < advertisedDevice.getServiceUUIDCount(); i++) {
           Serial.printf("\t\t%s\n", advertisedDevice.getServiceUUID(i).toString().c_str());
        }
        Serial.printf("\tHas our service: %s\n\n", advertisedDevice.isAdvertisingService(SERVICE_UUID) ? "True" : "False");
        
        // We have found a device, let us now see if it contains the service we are looking for.
        if (advertisedDevice.haveServiceUUID() && 
        advertisedDevice.isAdvertisingService(SERVICE_UUID) &&
        advertisedDevice.getName() == BLE_BROADCAST_NAME.c_str()) {
            BLEDevice::getScan()->stop();
            bleRemoteServer = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;
        }
    }     
};        

///////////////////////////////////////////////////////////////
// Put your setup code here, to run once
///////////////////////////////////////////////////////////////
void setup()
{
    // Init device
    M5.begin();
    M5.Lcd.setTextSize(3);

    BLEDevice::init("");

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}

///////////////////////////////////////////////////////////////
// Put your main code here, to run repeatedly
///////////////////////////////////////////////////////////////
void loop()
{
    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be true.
    if (doConnect == true)
    {
        if (connectToServer())
            Serial.println("We are now connected to the BLE Server.");
        else
            Serial.println("We have failed to connect to the server; there is nothin more we will do.");
        doConnect = false;
    }

    // If we are connected to a peer BLE Server, update the characteristic each time we are reached
    // with the current time since boot.
    if (deviceConnected)
    {
        // Format string to send to server
        String newValue = "Time since boot: " + String(millis() / 1000);
        Serial.println("Setting new characteristic value to \"" + newValue + "\"");

        // Set the characteristic's value to be the array of bytes that is actually a string.
        bleRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
        drawScreenTextWithBackground("Wrote value to server: " + String(newValue.c_str()), TFT_YELLOW); // Give feedback on screen
    }
    else if (doScan)
        BLEDevice::getScan()->start(0); // this is just example to start scan after disconnect, most likely there is better way to do it in arduino

    delay(1000); // Delay a second between loops.
}

///////////////////////////////////////////////////////////////
// Colors the background and then writes the text on top
///////////////////////////////////////////////////////////////
void drawScreenTextWithBackground(String text, int backgroundColor) {
    M5.Lcd.fillScreen(backgroundColor);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.println(text);
}