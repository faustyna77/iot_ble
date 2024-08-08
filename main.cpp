#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32Servo.h>


BLEServer* pServer = NULL;
BLECharacteristic* pSensorCharacteristic = NULL;
BLECharacteristic* pLedCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

const int ledPin = 2; // GPIO FROM ESP32 TO SETUP
const int ledPin3 = 3;
const int ledPin4 = 4;
const int ledPin5 = 5;
const int servoPin = 22;

const int servoPin2 = 23;
const int servoPin3 = 18;
const int servoPin4 = 19;
 Servo servo;
 Servo servo2;
 Servo servo3;
 Servo servo4;
 
// ANOTHER UUID FOR EVERY EVENTS 
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define LED_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pLedCharacteristic) {
        std::string value = pLedCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.print("Characteristic event, written: ");
            Serial.println(static_cast<int>(value[0])); // Print the integer value

            int receivedValue = static_cast<int>(value[0]);
            if (receivedValue == 1) {
                digitalWrite(ledPin, HIGH);
            } else if (receivedValue == 0) {
                digitalWrite(ledPin, LOW);
            }else if (receivedValue == 2) {
                digitalWrite(ledPin3, HIGH);
            }else if (receivedValue == 3) {
                digitalWrite(ledPin3, LOW);
            }else if (receivedValue == 4) {
                digitalWrite(ledPin4, HIGH);
            }else if (receivedValue == 5) {
                digitalWrite(ledPin3, LOW);
            }else if (receivedValue == 6) {
               servo.attach(servoPin);
                 for(int pos = 0; pos <= 180; pos += 1){
      servo.write(pos);
      delay(15);
    }
            }else if (receivedValue == 7) {
               servo.attach(servoPin);
                 for(int pos = 180; pos >= 0; pos -= 1){
      servo.write(pos);
      delay(15);
    }
            }else if (receivedValue == 8) {
               servo2.attach(servoPin2);
                 for(int pos = 0; pos <= 180; pos += 1){
      servo2.write(pos);
      delay(15);
    }
            }else if (receivedValue == 9) {
               servo2.attach(servoPin2);
                 for(int pos = 180; pos >= 0; pos -= 1){
      servo2.write(pos);
      delay(15);
    }
            }else if (receivedValue == 10) {
               servo3.attach(servoPin3);
                 for(int pos = 0; pos <= 180; pos += 1){
      servo3.write(pos);
      delay(15);
    }
            }else if (receivedValue == 11) {
               servo3.attach(servoPin3);
                 for(int pos = 180; pos >= 0; pos -= 1){
      servo3.write(pos);
      delay(15);
    }
            }else if (receivedValue == 12) {
               servo4.attach(servoPin4);
                 for(int pos = 0; pos <= 180; pos += 1){
      servo4.write(pos);
      delay(15);
    }
            }else if (receivedValue == 13) {
               servo4.attach(servoPin4);
                 for(int pos = 180; pos >= 0; pos -= 1){
      servo4.write(pos);
      delay(15);
    }
            }
        }
    }
};

void setup() {
  Serial.begin(9600);
  Servo servo;
  Servo servo2;
  Servo servo3;
  Servo servo4;
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  pinMode(ledPin5, OUTPUT);
  servo.attach(servoPin);
  servo2.attach(servoPin2);
  servo3.attach(servoPin3);
  servo4.attach(servoPin4);


  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pSensorCharacteristic = pService->createCharacteristic(
                      SENSOR_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Create the ON button Characteristic
  pLedCharacteristic = pService->createCharacteristic(
                      LED_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  // Register the callback for the ON button characteristic
  pLedCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pSensorCharacteristic->addDescriptor(new BLE2902());
  pLedCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // notify changed value
    if (deviceConnected) {
        pSensorCharacteristic->setValue(String(value).c_str());
        pSensorCharacteristic->notify();
        value++;
        Serial.print("New value notified: ");
        Serial.println(value);
        delay(3000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        Serial.println("Device disconnected.");
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
        Serial.println("Device Connected");
    }
}
