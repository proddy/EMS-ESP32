#ifndef UploadFileService_h
#define UploadFileService_h

#include "SecurityManager.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Update.h>
#include <WiFi.h>

#include <array>
#include <cstring>

#define UPLOAD_FILE_PATH "/rest/uploadFile"
#define TEMP_FILENAME_PATH "/pre_load.json" // for uploaded json files, handled by System::check_restore()

// Firmware validation constants
#define ESP_MAGIC_BYTE 0xE9
#define ESP32_CHIP_ID 0
#define ESP32S2_CHIP_ID 2
#define ESP32C3_CHIP_ID 5
#define ESP32S3_CHIP_ID 9
#define ESP32C6_CHIP_ID 13

// File size thresholds
#define MIN_FIRMWARE_SIZE 1000000

// Authentication state stored in request's _tempObject
struct UploadState {
    bool authenticated;
};

class UploadFileService {
  public:
    UploadFileService(AsyncWebServer * server, SecurityManager * securityManager);

  private:
    SecurityManager *    _securityManager;
    bool                 _is_firmware;
    std::array<char, 33> _md5;

    void handleUpload(AsyncWebServerRequest * request, const std::string & filename, size_t index, uint8_t * data, size_t len, bool final);
    void uploadComplete(AsyncWebServerRequest * request);
    void handleError(AsyncWebServerRequest * request, int code);
    void handleEarlyDisconnect();

    static inline bool validateFirmwareHeader(const uint8_t * data, size_t len);
    static inline bool hasMD5(const std::array<char, 33> & md5);
};

#endif