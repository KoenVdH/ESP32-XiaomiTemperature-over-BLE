/*
     Based on Alexey Shkurko: https://habr.com/ru/post/500208/ and https://habr.com/ru/post/501250/
*/

#include <M5Stack.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 20; //In seconds
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  
    uint8_t* findServiceData(uint8_t* data, size_t length, uint8_t* foundBlockLength) {
        uint8_t* rightBorder = data + length;
        while (data < rightBorder) {
            uint8_t blockLength = *data;
            if (blockLength < 5) {
                data += (blockLength+1);
                continue;
            }
            uint8_t blockType = *(data+1);
            uint16_t serviceType = *(uint16_t*)(data + 2);
            if (blockType == 0x16 && serviceType == 0xfe95) {
                *foundBlockLength = blockLength-3;
                return data+4;
            }
            data += (blockLength+1);
        }   
        return nullptr;
    }
  
    void onResult(BLEAdvertisedDevice advertisedDevice) {  
      
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

      // following line is commented out since sometimes the packets or not complete?
      //if (!advertisedDevice.getName().compare("MJ_HT_V1")){ 
      if (true){ 
        //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
        
        //Serial.println("");
        //Serial.printf("Device Name: %s, TX: %d, RSSI: %d", advertisedDevice.getName().c_str(), advertisedDevice.getTXPower(), advertisedDevice.getRSSI());
        //Serial.println("");
        //if (advertisedDevice.haveServiceUUID()) Serial.printf("UUID: %s", advertisedDevice.getServiceUUID().toString().c_str());
        //Serial.println("");
        //Serial.printf("Payloadlength: %d", advertisedDevice.getPayloadLength());
        //Serial.println("");
        //Serial.printf("Payload: %s", advertisedDevice.getPayload());
        //Serial.println("");
        //Serial.println("---------------------------------------------------------------------------------------");

        uint8_t* payload = advertisedDevice.getPayload();
        size_t payloadLength = advertisedDevice.getPayloadLength();
        //Serial.printf("\n\nAdvertised Device: %s\n", advertisedDevice.toString().c_str());
        uint8_t serviceDataLength=0;
        uint8_t* serviceData = findServiceData(payload, payloadLength, &serviceDataLength);

        if (serviceData == nullptr) {
            return;
        }

        Serial.printf("Found service data len: %d\n", serviceDataLength);

        switch (serviceData[11])
        {
            case 0x0D:
            {
                float temp = *(uint16_t*)(serviceData + 11 + 3) / 10.0;
                float humidity = *(uint16_t*)(serviceData + 11 + 5) / 10.0;
                Serial.printf("Temp: %.1f Humidity: %.1f\n", temp, humidity);
                M5.Lcd.printf("T: %.1f H: %.1f\n", temp, humidity);
            }
            break;
            case 0x04:
            {
                float temp = *(uint16_t*)(serviceData + 11 + 3) / 10.0;
                Serial.printf("Temp: %f\n", temp);
            }
            break;
            case 0x06:
            {
                float humidity = *(uint16_t*)(serviceData + 11 + 3) / 10.0;
                Serial.printf("Humidity: %f\n", humidity);
            }
            break;
            case 0x0A:
            {
                int battery = *(serviceData + 11 + 3);
                Serial.printf("Battery: %d\n", battery);
            }
            break;
        default:
            break;
        } 
    }     
   }
};

void setup() {
  M5.begin();
  Wire.begin();
  M5.Power.begin();

  // text print
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(3);
  
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() { 
  // put your main code here, to run repeatedly:
  M5.Lcd.fillScreen(BLACK);
  
  M5.Lcd.setTextSize(2);
  M5.Lcd.fillRect(0,180,360,60,0);
  uint8_t bat = M5.Power.getBatteryLevel();
  M5.Lcd.setCursor(0,180);
  if (M5.Power.isCharging()) M5.Lcd.printf("Battery is charging\r\n");
  else M5.Lcd.printf("Battery is not charging\r\n");
  M5.Lcd.printf("Battery Level %d", bat);
  M5.Lcd.progressBar(0, 220, 360, 20, bat);

  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(0, 0);
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  Serial.println("--");

  
}
