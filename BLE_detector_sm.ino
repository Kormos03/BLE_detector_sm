#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <BLEDevice.h>            // sets up BLE device constructs
#include <BLEScan.h>              // contains BLE scanning functions
#include <BLEAdvertisedDevice.h>  // contains BLE device characteristic data

int8_t scanTime = 2; //In seconds

// Server address
const char * serverUrl = "http://your-server-address:server-port";  // Change this to your server address

// WiFi credentials
const char* ssid = "SSID";       // Replace with your WiFi network name
const char* password = "WifiPass"; // Replace with your WiFi password

BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
      String strName;
      strName = advertisedDevice.getName().c_str();
      if ( strName.length() > 0 ) {
          Serial.printf("Name: %s n", advertisedDevice.getName().c_str());
          Serial.printf("Advertised Device: %s n", advertisedDevice.toString().c_str());
      }
  }
};

void sendToServer(String jsonData){
  HTTPClient http;
  delay(1000);
  http.begin(serverUrl);   // Begin HTTP request
  
  delay(100);
  http.addHeader("Content-Type", "application/json");
  delay(100);
  
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
  delay(1000);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("WiFi connected");

  delay(5000);

  // Start BLE scanning
  Serial.println("Scanning...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); // Create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // Active scan uses more power, but gets results faster
  pBLEScan->setInterval(50);
  pBLEScan->setWindow(49);  // Less or equal to setInterval value
}

void loop() {
  delay(200);
  
  // Perform BLE scan
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
  pBLEScan->clearResults();   // Delete results from BLEScan buffer to release memory
  delay(2500);
}
