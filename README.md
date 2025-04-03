# ESP32-C3 BLE Scanning and Server Communication Setup for collecting BLE devices and sendig to a server

This guide will help you set up an ESP32-C3 with BLE scanning capabilities and send the collected data to a server.

## 1. Hardware Requirements

- **ESP32-C3 Board** (In the Arduino IDE, select **ESP32C3 Dev Module**)
- **USB-C cable** for connection

## 2. Arduino IDE Setup

### 2.1. Add the ESP32 Board URL
- Open the Arduino IDE and go to **File > Preferences**.
- In the **Additional Boards Manager URLs** field, add the following URL: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

### 2.2. Board Setup in Arduino IDE
- Go to **Tools > Board** and select **ESP32C3 Dev Module**.
- Set the following settings:
- **Upload Speed**: `115200`
- **Flash Mode**: `QIO`
- **Flash Size**: `4MB`
- **Partition Scheme**: `Huge APP (no OTA, 1MB SPIFFS)`
- **Erase All Flash Before Upload**: Enable
- **Port**: Select the correct port for your ESP32-C3.

## 3. Code Setup

### 3.1. Paste the Code Below
Copy the following code and paste it into your Arduino IDE:

```cpp
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
```
### 3.2. Set Wi-Fi Credentials and Server URL

Before uploading the code, replace the following placeholders:

- **SSID**: Replace with your Wi-Fi network name.
- **WifiPass**: Replace with your Wi-Fi password.
- **serverUrl**: Replace with the URL of your server, e.g., `http://192.168.0.1:5000`.

 ### 4. Verify and Upload Code to the ESP32

1. Open the **Serial Monitor** from **Tools > Serial Monitor**.

2. Set the **Baud Rate** to **115200**.

3. Click **Upload** in the Arduino IDE to upload the code to the ESP32.

### 5. Monitor BLE Scan Results

1. After the code is uploaded and running, you will see BLE scanning results in the **Serial Monitor**.

2. The ESP32 will send the collected BLE data (MAC addresses and RSSI values) to your specified server as a JSON payload.

### 6. Server Setup

1. Ensure that your server is capable of receiving the **POST** requests and processing the JSON data.

2. You can use a Python server (e.g., **Flask** or **FastAPI**) or any other server framework to handle the incoming data.

---

### Troubleshooting

- **Ensure correct Wi-Fi credentials**: If the ESP32 cannot connect to Wi-Fi, double-check the SSID and password.
  
- **Server URL issues**: Make sure the server URL is correct and reachable from the ESP32.
  
- **BLE devices not detected**: Ensure that BLE devices are within range of the ESP32.
