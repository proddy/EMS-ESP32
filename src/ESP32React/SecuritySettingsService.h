#ifndef SecuritySettingsService_h
#define SecuritySettingsService_h

#include "SecurityManager.h"
#include "HttpEndpoint.h"
#include "FSPersistence.h"

// Use std::hash for std::string (no custom hash needed)

#ifndef FACTORY_ADMIN_USERNAME
#define FACTORY_ADMIN_USERNAME "admin"
#endif

#ifndef FACTORY_ADMIN_PASSWORD
#define FACTORY_ADMIN_PASSWORD "admin"
#endif

#ifndef FACTORY_GUEST_USERNAME
#define FACTORY_GUEST_USERNAME "guest"
#endif

#ifndef FACTORY_GUEST_PASSWORD
#define FACTORY_GUEST_PASSWORD "guest"
#endif

#define SECURITY_SETTINGS_FILE "/config/securitySettings.json"
#define SECURITY_SETTINGS_PATH "/rest/securitySettings"

#define GENERATE_TOKEN_SIZE 512
#define GENERATE_TOKEN_PATH "/rest/generateToken"

class SecuritySettings {
  public:
    std::string       jwtSecret;
    std::vector<User> users;

    static void read(SecuritySettings & settings, JsonObject root) {
        // secret
        root["jwt_secret"] = settings.jwtSecret.c_str();

        // users
        JsonArray users = root["users"].to<JsonArray>();
        for (const User & user : settings.users) {
            JsonObject userRoot  = users.add<JsonObject>();
            userRoot["username"] = user.username.c_str();
            userRoot["password"] = user.password.c_str();
            userRoot["admin"]    = user.admin;
        }
    }

    static StateUpdateResult update(JsonObjectConst root, SecuritySettings & settings) {
        // secret
        const char * jwt_secret = root["jwt_secret"] | FACTORY_JWT_SECRET;
        settings.jwtSecret      = jwt_secret;

        // users
        settings.users.clear();
        if (root["users"].is<JsonArray>()) {
            JsonArrayConst users = root["users"].as<JsonArrayConst>();
            for (size_t i = 0; i < users.size(); i++) {
                JsonObjectConst user     = users[i].as<JsonObjectConst>();
                const char *    username = user["username"];
                const char *    password = user["password"];
                bool            admin    = user["admin"];
                settings.users.emplace_back(username ? username : "", password ? password : "", admin);
            }
        } else {
            settings.users.emplace_back(FACTORY_ADMIN_USERNAME, FACTORY_ADMIN_PASSWORD, true);
            settings.users.emplace_back(FACTORY_GUEST_USERNAME, FACTORY_GUEST_PASSWORD, false);
        }
        return StateUpdateResult::CHANGED;
    }
};

class SecuritySettingsService final : public StatefulService<SecuritySettings>, public SecurityManager {
  public:
    SecuritySettingsService(AsyncWebServer * server, FS * fs);

    void begin();

    Authentication  authenticate(const std::string & username, const std::string & password) override;
    Authentication  authenticateRequest(AsyncWebServerRequest * request) override;
    std::string     generateJWT(const User * user) override;
    ArRequestFilterFunction      filterRequest(AuthenticationPredicate predicate) override;
    ArRequestHandlerFunction     wrapRequest(ArRequestHandlerFunction onRequest, AuthenticationPredicate predicate) override;
    ArJsonRequestHandlerFunction wrapCallback(ArJsonRequestHandlerFunction callback, AuthenticationPredicate predicate) override;

  private:
    HttpEndpoint<SecuritySettings>  _httpEndpoint;
    FSPersistence<SecuritySettings> _fsPersistence;
    ArduinoJsonJWT                  _jwtHandler;

    // User lookup map for O(1) access
    std::unordered_map<std::string, const User *> _userLookup;

    void generateToken(AsyncWebServerRequest * request);

    void configureJWTHandler();
    void rebuildUserLookup(); // Rebuild the user lookup map

    Authentication authenticateJWT(const std::string & jwt); // Lookup the user by JWT

    inline boolean validatePayload(const JsonObject parsedPayload, const User * user) const; // Verify the payload is correct
};

#endif