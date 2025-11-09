#ifndef APStatus_h
#define APStatus_h

// Forward declarations
class AsyncWebServer;
class AsyncWebServerRequest;
class SecurityManager;
class APSettingsService;

#define AP_STATUS_SERVICE_PATH "/rest/apStatus"

class APStatus {
  public:
    APStatus(AsyncWebServer * server, SecurityManager * securityManager, APSettingsService * apSettingsService);

  private:
    APSettingsService * _apSettingsService;
    void                apStatus(AsyncWebServerRequest * request);
};

#endif
