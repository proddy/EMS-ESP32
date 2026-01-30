#include "ESP32React.h"

#include "WWWData.h" // include auto-generated static web resources

static constexpr const char CACHE_CONTROL[] = "public,max-age=60";

ESP32React::ESP32React(AsyncWebServer * server, FS * fs)
    : _securitySettingsService(server, fs)
    , _networkSettingsService(server, fs, &_securitySettingsService)
    , _wifiScanner(server, &_securitySettingsService)
    , _networkStatus(server, &_securitySettingsService)
    , _apSettingsService(server, fs, &_securitySettingsService)
    , _apStatus(server, &_securitySettingsService, &_apSettingsService)
    , _ntpSettingsService(server, fs, &_securitySettingsService)
    , _ntpStatus(server, &_securitySettingsService)
    , _uploadFileService(server, &_securitySettingsService)
    , _mqttSettingsService(server, fs, &_securitySettingsService)
    , _mqttStatus(server, &_mqttSettingsService, &_securitySettingsService)
    , _authenticationService(server, &_securitySettingsService) {
    //
    // Serve static web resources
    //

    ArRequestHandlerFunction indexHtmlHandler = nullptr;

    WWWData::registerRoutes([server, &indexHtmlHandler](const char * uri, const String & contentType, const uint8_t * content, size_t len, const String & hash) {
        String etag = "\"" + hash + "\""; // RFC9110: ETag must be enclosed in double quotes

        ArRequestHandlerFunction requestHandler = [contentType, content, len, etag](AsyncWebServerRequest * request) {
            if (request->header(asyncsrv::T_INM) == etag) {
                request->send(304);
                return;
            }

            AsyncWebServerResponse * response = request->beginResponse(200, contentType, content, len);
            response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip, false);
            response->addHeader(asyncsrv::T_ETag, etag, false);
            response->addHeader(asyncsrv::T_Cache_Control, CACHE_CONTROL, false);
            request->send(response);
        };

        server->on(uri, HTTP_GET, requestHandler);

        // Capture index.html handler to set onNotFound once after all routes are registered
        if (strcmp(uri, "/index.html") == 0) {
            indexHtmlHandler = requestHandler;
        }
    });

    // Set onNotFound handler once after all routes are registered
    // Serving non matching get requests with "/index.html"
    // OPTIONS get a straight up 200 response
    if (indexHtmlHandler != nullptr) {
        server->onNotFound([indexHtmlHandler](AsyncWebServerRequest * request) {
            if (request->method() == HTTP_GET) {
                indexHtmlHandler(request);
            } else if (request->method() == HTTP_OPTIONS) {
                request->send(200);
            } else {
                request->send(404); // not found
            }
        });
    }
}

void ESP32React::begin() {
    _networkSettingsService.begin();
    _networkSettingsService.read([&](NetworkSettings & networkSettings) {
        DefaultHeaders & defaultHeaders = DefaultHeaders::Instance();
        if (networkSettings.enableCORS) {
            defaultHeaders.addHeader(asyncsrv::T_CORS_ACAO, networkSettings.CORSOrigin);
            defaultHeaders.addHeader(asyncsrv::T_CORS_ACAH, "Accept, Content-Type, Authorization");
            defaultHeaders.addHeader(asyncsrv::T_CORS_ACAC, "true");
        }
        defaultHeaders.addHeader(asyncsrv::T_Server, networkSettings.hostname);
    });
    _apSettingsService.begin();
    _ntpSettingsService.begin();
    _mqttSettingsService.begin();
    _securitySettingsService.begin();
}

void ESP32React::loop() {
    _networkSettingsService.loop();
    _apSettingsService.loop();
    _mqttSettingsService.loop();
}