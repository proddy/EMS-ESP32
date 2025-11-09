#ifndef ArduinoJsonJWT_H
#define ArduinoJsonJWT_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include <libb64/cdecode.h>
#include <libb64/cencode.h>
#include <mbedtls/md.h>

#include <string>

class ArduinoJsonJWT {
  public:
    explicit ArduinoJsonJWT(std::string secret);

    void        setSecret(std::string secret);
    std::string getSecret() const;

    std::string buildJWT(JsonObject payload);
    void        parseJWT(const std::string & jwt, JsonDocument & jsonDocument);

  private:
    std::string _secret;

    std::string sign(const std::string & value);

    static std::string encode(const char * cstr, int len);
    static std::string decode(const std::string & value);

    static const std::string & getJWTHeader() {
        static const std::string JWT_HEADER = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";
        return JWT_HEADER;
    }
};

#endif