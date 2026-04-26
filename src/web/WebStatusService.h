#ifndef WebStatusService_h
#define WebStatusService_h

#define EMSESP_SYSTEM_STATUS_SERVICE_PATH "/rest/systemStatus"
#define EMSESP_ACTION_SERVICE_PATH "/rest/action"

#include "../core/EMSESP_Version.h"
#include "../emsesp_version.h"

namespace emsesp {

class WebStatusService {
  public:
    WebStatusService(AsyncWebServer * server, SecurityManager * securityManager);
    void set_current_version(const std::string & version) {
        current_version_s = version;
    }
    std::string get_current_version() {
        return current_version_s;
    }

    // called from EMSESP::loop() to refresh the cached versions.json from emsesp.org so that the web
    // request handler never has to do blocking HTTPS on the small AsyncTCP stack
    void loop();

    // true once we've had at least one successful versions.json fetch
    bool versions_cache_valid() const {
        return versions_cache_valid_;
    }

    bool current_upgradeable() const; // true if a newer version is available

// make action function public so we can test in the debug and standalone mode
#ifndef EMSESP_STANDALONE
  protected:
#endif
    void systemStatus(AsyncWebServerRequest * request);
    void action(AsyncWebServerRequest * request, JsonVariant json);

  private:
    SecurityManager * _securityManager;

    // actions
    void    getVersions(JsonObject root);
    bool    exportData(JsonObject root, std::string & type);
    bool    getCustomSupport(JsonObject root);
    bool    uploadURL(const char * url);
    bool    setSystemStatus(const char * status);
    void    allvalues(JsonObject output);
    uint8_t upgradeImportantMessages(std::string & version);

    std::string current_version_s = EMSESP_APP_VERSION;

    // cached emsesp.org/versions.json. Refreshed from the main loop task, which has more stack.
    struct VersionInfo {
        std::string version;
        std::string date;
        bool        upgradeable = false;
    };
    VersionInfo versions_stable_;
    VersionInfo versions_dev_;
    bool        versions_cache_valid_   = false; // true once we've had at least one successful fetch
    uint32_t    versions_next_fetch_ms_ = 0;     // uuid::get_uptime() of the next attempt; 0 = ASAP

    bool refresh_versions_cache(); // does the actual HTTPS fetch + parse, returns true on success

    static constexpr uint32_t VERSIONS_REFRESH_INTERVAL_MS = 60UL * 60UL * 1000UL; // 1 hour on success
    static constexpr uint32_t VERSIONS_RETRY_INTERVAL_MS   = 5UL * 60UL * 1000UL;  // 5 min after failure
    static constexpr uint32_t VERSIONS_INITIAL_DELAY_MS    = 30UL * 1000UL;        // wait 30s after boot
};

} // namespace emsesp

#endif
