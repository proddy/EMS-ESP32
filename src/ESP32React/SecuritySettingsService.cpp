#include "SecuritySettingsService.h"

SecuritySettingsService::SecuritySettingsService(AsyncWebServer * server, FS * fs)
    : _httpEndpoint(SecuritySettings::read, SecuritySettings::update, this, server, SECURITY_SETTINGS_PATH, this)
    , _fsPersistence(SecuritySettings::read, SecuritySettings::update, this, fs, SECURITY_SETTINGS_FILE)
    , _jwtHandler(std::string(FACTORY_JWT_SECRET)) {
    addUpdateHandler([this] {
        configureJWTHandler();
        rebuildUserLookup();
    }, false);

    server->on(GENERATE_TOKEN_PATH,
               HTTP_GET,
               SecuritySettingsService::wrapRequest([this](AsyncWebServerRequest * request) { generateToken(request); }, AuthenticationPredicates::IS_ADMIN));
}

void SecuritySettingsService::begin() {
    _fsPersistence.readFromFS();
    configureJWTHandler();
    rebuildUserLookup();
}

Authentication SecuritySettingsService::authenticateRequest(AsyncWebServerRequest * request) {
    auto authorizationHeader = request->getHeader(AUTHORIZATION_HEADER);
    if (authorizationHeader) {
        const String & value = authorizationHeader->value();
        if (value.startsWith(AUTHORIZATION_HEADER_PREFIX)) {
            // Extract JWT and convert to std::string - substring creates temporary String
            std::string jwt(value.substring(AUTHORIZATION_HEADER_PREFIX_LEN).c_str());
            return authenticateJWT(jwt);
        }
    } else if (request->hasParam(ACCESS_TOKEN_PARAMATER)) {
        auto        tokenParamater = request->getParam(ACCESS_TOKEN_PARAMATER);
        std::string jwt(tokenParamater->value().c_str());
        return authenticateJWT(jwt);
    }
    return {};
}

void SecuritySettingsService::configureJWTHandler() {
    _jwtHandler.setSecret(_state.jwtSecret);
}

void SecuritySettingsService::rebuildUserLookup() {
    _userLookup.clear();
    _userLookup.reserve(_state.users.size());
    for (const User & user : _state.users) {
        _userLookup[user.username] = &user;
    }
}

Authentication SecuritySettingsService::authenticateJWT(const std::string & jwt) {
    JsonDocument payloadDocument;
    _jwtHandler.parseJWT(jwt, payloadDocument);
    if (payloadDocument.is<JsonObject>()) {
        JsonObject parsedPayload = payloadDocument.as<JsonObject>();
        const char * username_cstr = parsedPayload["username"];
        if (username_cstr) {
            std::string username = username_cstr;
            // O(1) lookup instead of O(n) iteration
            auto it = _userLookup.find(username);
            if (it != _userLookup.end() && validatePayload(parsedPayload, it->second)) {
                return Authentication(*it->second);
            }
        }
    }
    return {};
}

Authentication SecuritySettingsService::authenticate(const std::string & username, const std::string & password) {
    // O(1) lookup instead of O(n) iteration
    auto it = _userLookup.find(username);
    if (it != _userLookup.end() && it->second->password == password) {
        return Authentication(*it->second);
    }
    return {};
}

inline void populateJWTPayload(JsonObject payload, const User * user) {
    payload["username"] = user->username.c_str();
    payload["admin"]    = user->admin;
}

boolean SecuritySettingsService::validatePayload(const JsonObject parsedPayload, const User * user) const {
    // Direct field comparison is much faster than creating a JsonDocument
    const char * username_cstr = parsedPayload["username"];
    return username_cstr && std::string(username_cstr) == user->username && parsedPayload["admin"].as<bool>() == user->admin;
}

std::string SecuritySettingsService::generateJWT(const User * user) {
    JsonDocument jsonDocument;
    JsonObject   payload = jsonDocument.to<JsonObject>();
    populateJWTPayload(payload, user);
    return _jwtHandler.buildJWT(payload);
}

ArRequestFilterFunction SecuritySettingsService::filterRequest(AuthenticationPredicate predicate) {
    return [this, predicate](AsyncWebServerRequest * request) {
        Authentication authentication = authenticateRequest(request);
        return predicate(authentication);
    };
}

ArRequestHandlerFunction SecuritySettingsService::wrapRequest(ArRequestHandlerFunction onRequest, AuthenticationPredicate predicate) {
    return [this, onRequest, predicate](AsyncWebServerRequest * request) {
        Authentication authentication = authenticateRequest(request);
        if (!predicate(authentication)) {
            request->send(401);
            return;
        }
        onRequest(request);
    };
}

ArJsonRequestHandlerFunction SecuritySettingsService::wrapCallback(ArJsonRequestHandlerFunction onRequest, AuthenticationPredicate predicate) {
    return [this, onRequest, predicate](AsyncWebServerRequest * request, JsonVariant json) {
        Authentication authentication = authenticateRequest(request);
        if (!predicate(authentication)) {
            request->send(401);
            return;
        }
        onRequest(request, json);
    };
}

void SecuritySettingsService::generateToken(AsyncWebServerRequest * request) {
    auto usernameParam = request->getParam("username");
    
    // O(1) lookup instead of O(n) iteration
    std::string username(usernameParam->value().c_str());
    auto        it = _userLookup.find(username);
    if (it != _userLookup.end()) {
        auto *     response = new AsyncJsonResponse(false);
        JsonObject root     = response->getRoot();
        root["token"]       = generateJWT(it->second).c_str();
        response->setLength();
        request->send(response);
        return;
    }
    request->send(401);
}
