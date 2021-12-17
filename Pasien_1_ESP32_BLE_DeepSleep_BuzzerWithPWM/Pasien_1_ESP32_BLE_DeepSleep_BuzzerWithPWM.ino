#include <Wire.h>
#include "MAX30102_lib_intg.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 5        /* Time ESP32 will sleeping (in minutes) */
#define time_to_ON    30        /* in minute, interval time ESP32 ON     */
#define BUTTON_PIN_BITMASK 0x8004 // GPIOs 2 and 15: 2^2 + 2^15 = 32772 -> Hex: 0x8004
#define SERVICE_UUID        "a18f324a-ec32-11eb-9a03-0242ac130003"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

RTC_DATA_ATTR int state = LOW;

/* PWM With Buzzer */
int frequency = 1000;// frekuensi 10kHz
int ledchannel = 0;  // channel 0 dari 16 channel tersedia
int resolution = 8; // 12 bit resolusi
const int buzzerPin = 5; // buzzer pin 5

/* Time to sleep */
const int ms_to_s = 1000;
const int s_to_m = 60;

unsigned long timePrevious = 0;
unsigned long timeNow = 0;

/* Devive use */
int GREEN_LED_PIN = 25;
int YELLOW_LED_PIN = 26;

/* BLE */
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data

double spo2,ratio,correl;  //SPO2 value
int8_t spo2_valid;  //indicator to show if the SPO2 calculation is valid
int32_t heart_rate; //heart rate value
int8_t hr_valid;  //indicator to show if the heart rate calculation is valid
double temp;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void BIP_Buzzer(int num_bip){
  int i;
  for(i=0;i<num_bip;i++){
    ledcWrite(ledchannel, 127);
    delay(200);
    ledcWrite(ledchannel, 0);
    delay(50);
  }

  /* let buzzer off again */
  ledcWrite(ledchannel, 0);
}

/* Define Lib Intg MAX30102 as particleSensor */
MAX30102_LIB_INTG particleSensor;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(GREEN_LED_PIN,OUTPUT);
  pinMode(YELLOW_LED_PIN,OUTPUT);

  ledcSetup(ledchannel, frequency, resolution);
  ledcAttachPin(buzzerPin, ledchannel); //PWM di pin 15
  
  while (!particleSensor.begin()) {
    Serial.println("MAX30102 was not found");
    delay(1000);
  }
  particleSensor.sensorConfiguration(/*ledBrightness=*/60, /*sampleAverage=*/SAMPLEAVG_8, \
                                  /*ledMode=*/MODE_MULTILED, /*sampleRate=*/SAMPLERATE_200, \
                                  /*pulseWidth=*/PULSEWIDTH_411, /*adcRange=*/ADCRANGE_16384);

  // Create the BLE Device
  BLEDevice::init("Pasien1");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x0);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("Esp32 is on or has woken up after deepsleep");
  BIP_Buzzer(1);
}

void loop() {
  timeNow = millis();
  
  /* Memanggil function untuk membaca data pasien menggunakan sensor MAX30102 */
  particleSensor.heartrateAndOxygenSaturation(&spo2, &spo2_valid, &heart_rate, &hr_valid, &ratio, &correl);
  temp = particleSensor.readTemperatureC();
  int spo2fix=round(spo2)+2;

  /* Menggabungkan seluruh data pasien untuk dikirimkan */
  String sendToESP = "";
  sendToESP += spo2fix;
  sendToESP += ";";
  sendToESP += heart_rate;
  sendToESP += ";";
  sendToESP += 1;
  sendToESP += ";";
  sendToESP += temp;
  sendToESP += '\0';
    
  // SpO2 dan HR valid
  if (spo2_valid == 1 && hr_valid == 1) {
      
    if (deviceConnected){
      char txString[16];
      sendToESP.toCharArray(txString,16);
     
      pCharacteristic->setValue(txString);
      pCharacteristic->notify();

      delay(500);
    }
    /* Sending data to ESP32 dengan serial UART / RX TX */
    Serial.println(sendToESP);

    /* Bila parameter pasien: normal, buzzer 2 kali bip sound */
    if (spo2fix>95 && heart_rate>65 && heart_rate<100){
      BIP_Buzzer(2);
    }
    /* Bila parameter pasien: tidak normal, buzzer 3 kali bip sound */
    else if(spo2fix<95 || heart_rate<65 || heart_rate>100){
      BIP_Buzzer(3);
    }
  }
  
  /* Jika BLE Disconnecting */
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    oldDeviceConnected = deviceConnected;
  }
  /* Bila BLE Connecting */
  if (deviceConnected && !oldDeviceConnected) {
    /* do stuff here on connecting */
    oldDeviceConnected = deviceConnected;
  }
  
  /* Going to Deep Sleep if device on with ... minutes */
  if (timeNow - timePrevious >= time_to_ON * ms_to_s * s_to_m){    
    Serial.println("ESP32 is Going to Deep Sleep");
    timePrevious = millis();
    delay(100);

    /* Bila button wakeup ditekan, maka ESP32 akan menyala */
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    /* Bila button wakeup tidak ditekan, maka ESP32 akan menyala sendirinya setelah 5 menit kemudian */ 
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR * s_to_m);       

    /* Power Mode: DeepSleep */
    esp_deep_sleep_start();
  }
}
