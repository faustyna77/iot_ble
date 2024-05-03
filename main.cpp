#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32Servo.h>

BLEServer* pServer = NULL;
BLECharacteristic* pSensorCharacteristic = NULL;
BLECharacteristic* pLedCharacteristic = NULL;
BLECharacteristic* pservoCharacteristic=NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

const int ledPin2 = 2;
const int ledPin4 = 4;
const int ledPin5 = 5;
const int ledPin6=19;
const int servoPin = 21;

Servo servo;

#define SERVICE_UUID "19b10000-e8f2-537e-4f6c-d104768a1214"
#define SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define LED_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"
#define SERVO_CHARACTERISTIC_UUID "e42dc8ca-8171-4775-98f6-7fdaa333044b"

//void servoPos();
//void servoStop();

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCharacteristicCallbacksLed : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pLedCharacteristic) {
    std::string value = pLedCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.print("Characteristic event, written: ");
      Serial.println(static_cast<int>(value[0]));

      int receivedValue = static_cast<int>(value[0]);
      if (receivedValue ==1) {
        digitalWrite(ledPin2, HIGH);
      }else if(receivedValue ==0){
        digitalWrite(ledPin2, LOW);

      }else if(receivedValue ==2){
        digitalWrite(ledPin4, HIGH);
       
    
      } else if (receivedValue == 3) {
        digitalWrite(ledPin4, LOW);
      } else if (receivedValue == 4) {
        
          digitalWrite(ledPin5, HIGH);
       
      }else if(receivedValue==5)
      {
        digitalWrite(ledPin5, LOW);
      }else if(receivedValue==6)
      {

        
    digitalWrite(ledPin6,HIGH);
       

      }else if(receivedValue==11)
      {
        digitalWrite(ledPin6,LOW);
      }

      
    }
  }
};



class MyCharacteristicCallbacksServo : public BLECharacteristicCallbacks {
  void onWriteServo(BLECharacteristic* pservoCharacteristic) {
    std::string value = pservoCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.print("Characteristic event, written: ");
      Serial.println(static_cast<int>(value[0]));

      int receivedValue = static_cast<int>(value[0]);
      if (receivedValue ==12) {
          servo.write(180);
    delay(200);
    servo.write(0);
    //delay(25);
        
      }

      
    }
  }
};

void setup() {
  Serial.begin(9600);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  pinMode(ledPin5, OUTPUT);
  pinMode(ledPin6,OUTPUT);
  servo.attach(servoPin);

  

  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());


  BLEService* pService = pServer->createService(SERVICE_UUID);

  pSensorCharacteristic = pService->createCharacteristic(
    SENSOR_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_INDICATE
  );

  pLedCharacteristic = pService->createCharacteristic(
    LED_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );

   pservoCharacteristic= pService->createCharacteristic(
    SERVO_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );


  pLedCharacteristic->setCallbacks(new MyCharacteristicCallbacksLed());
   pSensorCharacteristic->setCallbacks(new MyCharacteristicCallbacksServo());

  pSensorCharacteristic->addDescriptor(new BLE2902());
  pLedCharacteristic->addDescriptor(new BLE2902());
  pservoCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    pSensorCharacteristic->setValue(String(value).c_str());
    pSensorCharacteristic->notify();
    value++;
    Serial.print("New value notified: ");
    Serial.println(value);
    delay(3000);
  }
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected.");
    delay(500);
    pServer->startAdvertising();
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected");
  }
}






