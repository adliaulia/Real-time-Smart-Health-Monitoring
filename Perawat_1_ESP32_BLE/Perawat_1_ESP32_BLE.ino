#include<SPI.h>
#include<MFRC522.h>
#include "BLEDevice.h"
#include "BLEScan.h"
#include <string.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SS_PIN    33 
#define RST_PIN   25 
MFRC522 rfid(SS_PIN, RST_PIN);

int x;
bool connectRFID = false;

  int DataReceive1;
  int DataReceive2;
  int DataReceive3;
  float DataReceive4;

// The remote service we wish to connect to.
static BLEUUID pasiendefault("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID pasien1UUID("a18f324a-ec32-11eb-9a03-0242ac130003");

// The characteristic of the remote service we are interested in.
static BLEUUID Characteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8");

struct bledevice{
  String Nom;
  BLEUUID service;
  BLEUUID character;
};

bledevice bledevices[21]={
  {"4fafc201-1fb5-459e-8fcc-c5c9c331914b", pasiendefault, Characteristic},
  {"a18f324a-ec32-11eb-9a03-0242ac130003", pasien1UUID, Characteristic},
};

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.println((char*)pData);

  char tempChars[16];
  strncpy(tempChars, (char*)pData, 16);
  char * strtokIndx; // this is used by strtok() as an index
  
  strtokIndx = strtok(tempChars,";");      // get the first part - the string
  DataReceive1 = atoi(strtokIndx);
  
  strtokIndx = strtok(NULL, ";"); // this continues where the previous call left off
  DataReceive2 = atoi(strtokIndx);     // convert this part to an integer
  
  strtokIndx = strtok(NULL, ";");
  DataReceive3 = atoi(strtokIndx);     // convert this part to a int

  strtokIndx = strtok(NULL, ";");
  DataReceive4 = atof(strtokIndx);     // convert this part to a float
  
//  String sendToESP = String((char*)pData);
//  DataReceive1 = splitString(sendToESP, ';', 0, 3).toInt();
//  DataReceive2 = splitString(sendToESP, ';', 1, 3).toInt();
//  DataReceive3 = splitString(sendToESP, ';', 2, 3).toFloat();
//  DataReceive2 = splitString(sendToESP, ';', 3, 3).toInt();
  Serial.println(DataReceive1);
  Serial.println(DataReceive2);
  Serial.println(DataReceive3);
  Serial.println(DataReceive4);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
    ESP.restart();
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(bledevices[x].service);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(bledevices[x].service.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(bledevices[x].character);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(bledevices[1].character.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(bledevices[x].service)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  }// onResult
}; // MyAdvertisedDeviceCallbacks

/* -----------------------------------------   Main of loop   ----------------------------------------- */
void setup() {
  Serial.begin(115200);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  lcd.begin();                  
  lcd.backlight();
  
  cekRfid();
  bleScan(); 
}
/* -----------------------------------------   End of setup   ----------------------------------------- */

/* -----------------------------------------   Main of loop   ----------------------------------------- */
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } 
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tidak Terhubung!");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Silahkan Ulang ");
      lcd.setCursor(0, 1);
      lcd.print("    Kembali     ");
      delay(1000);
      ESP.restart();
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
  
  tampilanData();

  delay(1000); // Delay a second between loops.
} 
/* -----------------------------------------   End of loop   ----------------------------------------- */

/* -----------------------------------------                 ----------------------------------------- */

void cekRfid(){
  byte pasien1KeyUID[4] = {0xE9, 0x36, 0x3C, 0x98};
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tempelkan Kartu!");
  Serial.println("Scanning");
  while(connectRFID == false){
    
    if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      //Serial.println("masuk");
      if (rfid.uid.uidByte[0] == pasien1KeyUID[0] &&
          rfid.uid.uidByte[1] == pasien1KeyUID[1] &&
          rfid.uid.uidByte[2] == pasien1KeyUID[2] &&
          rfid.uid.uidByte[3] == pasien1KeyUID[3] ) {
        Serial.println("Access is granted for pasien1");
        x=1;
        connectRFID = true;
      }
      else
      {
        Serial.print("Access denied for user with UID:");
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(rfid.uid.uidByte[i], HEX);
        }
        Serial.println();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Kartu Tidak   ");
        lcd.setCursor(0, 1);
        lcd.print("   Terdaftar!   ");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" Silahkan Ulang ");
        lcd.setCursor(0, 1);
        lcd.print("    Kembali     ");
        delay(1000);
        cekRfid();
      }

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
 }
    
}

void bleScan(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Menunggu...");
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  //pBLEScan->start(5, false);
  pBLEScan->start(0); 
}

void tampilanData(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pasien ");
  lcd.setCursor(7, 0);
  lcd.print(x);
  
  lcd.setCursor(0, 1);
  lcd.print("HR:");
  lcd.setCursor(3, 1);
  lcd.print(DataReceive2, DEC);
  
  lcd.setCursor(8, 1);
  lcd.print("SpO2:");
  lcd.setCursor(13, 1);
  lcd.print(DataReceive1, DEC);
  lcd.print("%");
  
  lcd.setCursor(9, 0);
  lcd.print("T:");
  lcd.setCursor(11, 0);
  lcd.print(DataReceive4);
}
