#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SD_MMC.h>
#include <FS.h>
#include "LittleFS.h"
#include <BLEDevice.h>  // Include BLEDevice for scanning

const char* ssid = "cuidadoconlamina";
const char* password = "12345678";

#define SD_MMC_CMD 38 // Please do not modify it.
#define SD_MMC_CLK 39 // Please do not modify it.
#define SD_MMC_D0 40  // Please do not modify it.

AsyncWebServer server(80);
String btDevicesList = "[]";
String fileList = "[]";  // Store the list of files here

// SDMMC setup
bool initSDCard() {
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
  Serial.print("Attempt mount ");

  if (!SD_MMC.begin("/sdcard", true, true, SDMMC_FREQ_DEFAULT, 5)) {
    Serial.println("SD Card Mount Failed");
    return false;
  }
  Serial.println("SD Card mounted.");

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return false;
  }
  Serial.print("SD_MMC Card initialized ");
  return true;
}

// Scan SD card for files and store them in fileList
void scanFiles() {
  fileList = "[";  // Initialize an empty JSON array
  File root = SD_MMC.open("/");
  File file = root.openNextFile();
  while (file) {
    if (String(file.name()).endsWith(".mp3")) {
      if (fileList != "[") {
        fileList += ",";  // Add comma to separate file names
      }
      fileList += "\"" + String(file.name()) + "\"";  // Add the file name to the list
    }
    file = root.openNextFile();
  }
  fileList += "]";  // Close the JSON array
  Serial.println("File list: " + fileList);
}

// Bluetooth scan callback handler
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    String deviceName = advertisedDevice.getName().c_str();  // Get device name
    if (deviceName == "") {
      // Ignore unnamed devices to avoid cluttering the list and potential crashes
      Serial.println("Unnamed device found, skipping.");
      return;
    }
    
    if (btDevicesList != "[") {
      btDevicesList += ",";  // Add a comma if the list isn't empty
    }
    btDevicesList += "\"" + deviceName + "\"";  // Append the device name to the list

    Serial.println("Device found: " + deviceName);

    // Yield to watchdog to prevent triggering it during long scans
    delay(10);
  }
};

// Bluetooth scan function
void scanBluetoothDevices() {
  BLEDevice::init("");
  btDevicesList = "[";  // Reset the device list
  BLEScan* pBLEScan = BLEDevice::getScan(); // Create a BLE scan object
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
  pBLEScan->setActiveScan(true); // Active scan uses more power, but gets more data
  pBLEScan->start(3, false); // Scan for 3 seconds to reduce scan duration
  btDevicesList += "]";  // Close the JSON array
  pBLEScan->clearResults(); // Clear results after scanning
}

void handleUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  static File uploadFile;
  if (!index) {
    // Create/Open file for writing
    uploadFile = SD_MMC.open("/" + filename, FILE_WRITE);
    if (!uploadFile) {
      Serial.println("Failed to open file for writing");
      return;
    }
  }
  // Write received data to file
  if (uploadFile) {
    uploadFile.write(data, len);
  }
  if (final) {
    // Close file after complete upload
    if (uploadFile) {
      uploadFile.close();
    }
    // Rescan the files after upload completes
    scanFiles();
    request->send(200, "text/plain", "Upload Complete");
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi as an access point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(IP);

  // Initialize SD card with SD_MMC (SDMMC protocol)
  if (!initSDCard()) {
    Serial.println("SD Card initialization failed.");
    return;
  }

  // Scan files at startup
  scanFiles();

  // Initialize LittleFS for serving HTML/CSS files
  if (!LittleFS.begin(true)) {
    Serial.println("An error occurred while mounting LittleFS");
    return;
  }

  // Handle file upload
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){}, 
  handleUpload);

  // Serve the main page (upload and select page)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", String());
  });

  // Serve a page for file selection
  server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/files.html", String());
  });

  // Serve a page for Bluetooth device listing
  server.on("/speakers", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/speakers.html", String());
  });

  // List files API
  server.on("/list_files", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", fileList);  // Send the list of files as JSON
  });

  // List Bluetooth devices API
  server.on("/list_bt_devices", HTTP_GET, [](AsyncWebServerRequest *request) {
    scanBluetoothDevices();  // Trigger scan only when requested
    request->send(200, "application/json", btDevicesList);
  });

  // Start the server
  server.begin();
}

void loop() {
  // Main loop (can include audio playback logic based on selected file and speaker)
}
