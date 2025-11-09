#include "UploadFileService.h"

#include <emsesp.h>

#include <esp_app_format.h>

// Returns string view of extension without creating substring
static inline const char * getFilenameExtension(const std::string & filename) {
    const auto pos = filename.rfind('.');
    if (pos != std::string::npos && pos < filename.length() - 1) {
        return filename.c_str() + pos + 1;
    }
    return nullptr;
}

UploadFileService::UploadFileService(AsyncWebServer * server, SecurityManager * securityManager)
    : _securityManager(securityManager)
    , _is_firmware(false)
    , _md5() {
    _md5.fill(0); // Initialize MD5 buffer

    // upload a file via a form
    server->on(
        UPLOAD_FILE_PATH,
        HTTP_POST,
        [this](AsyncWebServerRequest * request) { uploadComplete(request); },
        [this](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t * data, size_t len, bool final) {
            handleUpload(request, std::string(filename.c_str()), index, data, len, final);
        });
}

// Validate firmware header based on chip type - extracted to reduce duplication
inline bool UploadFileService::validateFirmwareHeader(const uint8_t * data, size_t len) {
    if (len <= 12 || data[0] != ESP_MAGIC_BYTE) {
        return false;
    }

#if CONFIG_IDF_TARGET_ESP32
    return data[12] == ESP32_CHIP_ID;
#elif CONFIG_IDF_TARGET_ESP32S2
    return data[12] == ESP32S2_CHIP_ID;
#elif CONFIG_IDF_TARGET_ESP32C3
    return data[12] == ESP32C3_CHIP_ID;
#elif CONFIG_IDF_TARGET_ESP32S3
    return data[12] == ESP32S3_CHIP_ID;
#elif CONFIG_IDF_TARGET_ESP32C6
    return data[12] == ESP32C6_CHIP_ID;
#else
    return true; // Unknown target, allow through
#endif
}

// Check if MD5 is set
inline bool UploadFileService::hasMD5(const std::array<char, 33> & md5) {
    return md5[0] != '\0' && std::strlen(md5.data()) == 32;
}

void UploadFileService::handleUpload(AsyncWebServerRequest * request, const std::string & filename, size_t index, uint8_t * data, size_t len, bool final) {
    // Early exit if we already encountered an error
    if (request->_tempObject) {
        return;
    }

    // Authenticate ONLY on first chunk to avoid repeated expensive checks
    if (!index) {
        Authentication authentication = _securityManager->authenticateRequest(request);
        if (!AuthenticationPredicates::IS_ADMIN(authentication)) {
            handleError(request, 403); // send the forbidden response
            return;
        }

        // Check file extension
        const char * extension = getFilenameExtension(filename);
        if (!extension) {
            _md5[0] = '\0';
            handleError(request, 406); // Not Acceptable - no extension
            return;
        }

        const std::size_t filesize = request->contentLength();
        _is_firmware               = false;

        // Use strcmp for efficiency instead of String comparison
        if (std::strcmp(extension, "bin") == 0 && filesize > MIN_FIRMWARE_SIZE) {
            _is_firmware = true;
        } else if (std::strcmp(extension, "json") == 0) {
            _md5[0] = '\0'; // clear md5
        } else if (std::strcmp(extension, "md5") == 0) {
            // Handle MD5 file upload
            if (len == _md5.size() - 1) {
                std::memcpy(_md5.data(), data, _md5.size() - 1);
                _md5.back() = '\0';
            }
            return;
        } else {
            _md5[0] = '\0';
            handleError(request, 406); // Not Acceptable - unsupported file type
            return;
        }

        if (_is_firmware) {
            // Validate firmware header using extracted function
            if (!validateFirmwareHeader(data, len)) {
                handleError(request, 503); // service unavailable - wrong chip type
                return;
            }

            // Initialize the ArduinoOTA updater
            if (Update.begin(filesize - sizeof(esp_image_header_t))) {
                if (hasMD5(_md5)) {
                    Update.setMD5(_md5.data());
                    _md5[0] = '\0';
                }
                request->onDisconnect([this] { handleEarlyDisconnect(); });
            } else {
                handleError(request, 507); // Insufficient Storage
                return;
            }
        } else {
            // Open temp file for json content
            request->_tempFile = LittleFS.open(TEMP_FILENAME_PATH, "w");
            if (!request->_tempFile) {
                handleError(request, 507); // Failed to open file
                return;
            }
        }
    }

    // Process chunk data
    if (!_is_firmware) {
        // File upload - stream to filesystem
        if (len && len != request->_tempFile.write(data, len)) {
            handleError(request, 507); // Insufficient Storage
        }
    } else {
        // Firmware upload - stream to OTA updater
        if (Update.write(data, len) != len) {
            handleError(request, 500); // Write failed
            return;
        }
        if (final && !Update.end(true)) {
            handleError(request, 500); // Finalization failed
        }
    }
}

void UploadFileService::uploadComplete(AsyncWebServerRequest * request) {
    // Handle JSON file upload completion
    if (request->_tempFile) {
        request->_tempFile.close();
        request->send(200);
        emsesp::EMSESP::system_.systemStatus(emsesp::SYSTEM_STATUS::SYSTEM_STATUS_PENDING_RESTART);
        return;
    }

    // Handle firmware upgrade completion (no error occurred)
    if (_is_firmware && !request->_tempObject) {
        request->send(200);
        emsesp::EMSESP::system_.systemStatus(emsesp::SYSTEM_STATUS::SYSTEM_STATUS_PENDING_RESTART);
        return;
    }

    // Handle MD5 file upload response
    if (hasMD5(_md5)) {
        auto *     response = new AsyncJsonResponse(false);
        JsonObject root     = response->getRoot();
        root["md5"]         = _md5.data();
        response->setLength();
        request->send(response);
        return;
    }

    handleError(request, 500);
}

void UploadFileService::handleError(AsyncWebServerRequest * request, int code) {
    // if we have had an error already, do nothing
    if (request->_tempObject) {
        return;
    }

    // send the error code to the client and record the error code in the temp object
    AsyncWebServerResponse * response = request->beginResponse(code);
    request->send(response);

    // check for invalid extension and immediately kill the connection, which will through an error
    // that is caught by the web code. Unfortunately the http error code is not sent to the client on fast network connections
    if (code == 406) {
        request->client()->close(true);
        handleEarlyDisconnect();
    }
}

void UploadFileService::handleEarlyDisconnect() {
    _is_firmware = false;
    Update.abort();
}
