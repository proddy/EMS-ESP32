#ifndef SecurityManager_h
#define SecurityManager_h

#include "Arduino.h"
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"

#include <memory>
#include <string>
#include <unordered_map>

#define FACTORY_JWT_SECRET "ems-esp"
#define ACCESS_TOKEN_PARAMATER "access_token"
#define AUTHORIZATION_HEADER "Authorization"
#define AUTHORIZATION_HEADER_PREFIX "Bearer "
#define AUTHORIZATION_HEADER_PREFIX_LEN 7

#define MAX_JWT_SIZE 128

class User {
  public:
    std::string username;
    std::string password;
    bool        admin;

    User(std::string username, std::string password, bool admin)
        : username(std::move(username))
        , password(std::move(password))
        , admin(admin) {
    }

    // Default copy/move operations are fine for this class
    User(const User &) = default;
    User(User &&) noexcept = default;
    User & operator=(const User &) = default;
    User & operator=(User &&) noexcept = default;
};

class Authentication {
  public:
    std::unique_ptr<User> user;
    bool                  authenticated = false;

    explicit Authentication(const User & user)
        : user(std::make_unique<User>(user))
        , authenticated(true) {
    }

    Authentication() = default;

    // Move-only semantics (unique_ptr is not copyable)
    Authentication(Authentication &&) noexcept = default;
    Authentication & operator=(Authentication &&) noexcept = default;

    // Delete copy operations to prevent double-ownership
    Authentication(const Authentication &) = delete;
    Authentication & operator=(const Authentication &) = delete;

    ~Authentication() = default;
};

typedef std::function<bool(const Authentication & authentication)> AuthenticationPredicate;

class AuthenticationPredicates {
  public:
    static constexpr bool NONE_REQUIRED(const Authentication & authentication) {
        (void)authentication;
        return true;
    }
    static bool IS_AUTHENTICATED(const Authentication & authentication) {
        return authentication.authenticated;
    }
    static bool IS_ADMIN(const Authentication & authentication) {
        return authentication.authenticated && authentication.user && authentication.user->admin;
    }
};

class SecurityManager {
  public:
    virtual ~SecurityManager() = default;

    virtual Authentication               authenticateRequest(AsyncWebServerRequest * request)                                    = 0;
    virtual ArRequestFilterFunction      filterRequest(AuthenticationPredicate predicate)                                        = 0;
    virtual ArRequestHandlerFunction     wrapRequest(ArRequestHandlerFunction onRequest, AuthenticationPredicate predicate)      = 0;
    virtual ArJsonRequestHandlerFunction wrapCallback(ArJsonRequestHandlerFunction onRequest, AuthenticationPredicate predicate) = 0;

    void addEndpoint(AsyncWebServer *             server,
                     const String &               path,
                     AuthenticationPredicate      predicate,
                     ArJsonRequestHandlerFunction function,
                     WebRequestMethodComposite    method = HTTP_POST) {
    }

    // non-Json endpoints - default GET
    void addEndpoint(AsyncWebServer *          server,
                     const String &            path,
                     AuthenticationPredicate   predicate,
                     ArRequestHandlerFunction  function,
                     WebRequestMethodComposite method = HTTP_GET) {
    }
};

#endif
