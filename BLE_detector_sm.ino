#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <BLEDevice.h>            // sets up BLE device constructs
#include <BLEScan.h>              // contains BLE scanning functions
#include <BLEAdvertisedDevice.h>  // contains BLE device characteristic data

int8_t scanTime = 30; //In seconds

  //Server address
const char * serverUrl = "http://serverIP:serverPORT";

// WiFi credentials
const char* ssid = ""; // WiFi network name
const char* password = "";     // WiFi password
   
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 

{

    void onResult(BLEAdvertisedDevice advertisedDevice) 

    {

      String strName;
      strName = advertisedDevice.getName().c_str();
      if ( strName.length() > 0 )

      {
        Serial.printf("Name: %s n", advertisedDevice.getName().c_str());
        Serial.printf("Advertised Device: %s n", advertisedDevice.toString().c_str());
      }

    }

};

void sendToServer(String jsonData){
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, skipping HTTP request...");
        return;
    }

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    int8_t httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
        Serial.print("Server response: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
    } else {
        Serial.print("Error sending data: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}

void setup() {
  delay(100);
  Serial.begin(115200);
  delay(100);
  //connect to server via wifi
  WiFi.begin(ssid, password);
  
    //Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());

    delay(500);
    
    //start BLE
  Serial.println("Scanning...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(50);
  pBLEScan->setWindow(49);  // less or equal setInterval value

}

void loop() {
  delay(200);
  
  // BLE
BLEScanResults foundDevices = *pBLEScan->start(10, false);

    StaticJsonDocument<128> jsonDoc;
    JsonArray devicesArray = jsonDoc.createNestedArray("devices");

    for (int8_t i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        JsonObject deviceObj = devicesArray.createNestedObject();
        deviceObj["mac"] = device.getAddress().toString().c_str();
        deviceObj["rssi"] = device.getRSSI();
    }

    String jsonString;
    serializeJson(jsonDoc, jsonString);
   Serial.println("JSON Payload:");
   Serial.println(jsonString);

  sendToServer(jsonString);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(10000);


}
