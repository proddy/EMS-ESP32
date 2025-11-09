#include "ArduinoJsonJWT.h"

#include <algorithm>
#include <array>

ArduinoJsonJWT::ArduinoJsonJWT(std::string secret)
    : _secret(std::move(secret)) {
}

void ArduinoJsonJWT::setSecret(std::string secret) {
    _secret = std::move(secret);
}

std::string ArduinoJsonJWT::getSecret() const {
    return _secret;
}

std::string ArduinoJsonJWT::buildJWT(JsonObject payload) {
    // serialize, then encode payload
    std::string jwt;
    serializeJson(payload, jwt);
    jwt = encode(jwt.c_str(), static_cast<int>(jwt.length()));

    // add the header to payload
    jwt = getJWTHeader() + '.' + jwt;

    // add signature
    jwt += '.' + sign(jwt);

    return jwt;
}

void ArduinoJsonJWT::parseJWT(const std::string & jwt_input, JsonDocument & jsonDocument) {
    // clear json document before we begin, jsonDocument wil be null on failure
    jsonDocument.clear();

    const std::string & jwt_header      = getJWTHeader();
    const std::size_t   jwt_header_size = jwt_header.length();

    // must have the correct header and delimiter
    if (jwt_input.rfind(jwt_header, 0) != 0 || jwt_input.find('.') != jwt_header_size) {
        return;
    }

    // check there is a signature delimiter
    const std::size_t signatureDelimiterIndex = jwt_input.rfind('.');
    if (signatureDelimiterIndex == jwt_header_size || signatureDelimiterIndex == std::string::npos) {
        return;
    }

    // check the signature is valid
    const std::string signature = jwt_input.substr(signatureDelimiterIndex + 1);
    std::string       jwt       = jwt_input.substr(0, signatureDelimiterIndex);
    if (sign(jwt) != signature) {
        return;
    }

    // decode payload
    jwt = jwt.substr(jwt_header_size + 1);
    jwt = decode(jwt);

    // parse payload, clearing json document after failure
    const DeserializationError error = deserializeJson(jsonDocument, jwt);
    if (error != DeserializationError::Ok || !jsonDocument.is<JsonObject>()) {
        jsonDocument.clear();
    }
}

/*
 * ESP32 uses mbedtls, with decent HMAC implementations supporting sha256, as well as others.
 * No need to pull in additional crypto libraries - lets use what we already have.
 */
std::string ArduinoJsonJWT::sign(const std::string & payload) {
    std::array<unsigned char, 32> hmacResult{};
    {
        mbedtls_md_context_t ctx;
        mbedtls_md_type_t    md_type = MBEDTLS_MD_SHA256;
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
        mbedtls_md_hmac_starts(&ctx, reinterpret_cast<const unsigned char *>(_secret.c_str()), _secret.length());
        mbedtls_md_hmac_update(&ctx, reinterpret_cast<const unsigned char *>(payload.c_str()), payload.length());
        mbedtls_md_hmac_finish(&ctx, hmacResult.data());
        mbedtls_md_free(&ctx);
    }
    return encode(reinterpret_cast<const char *>(hmacResult.data()), hmacResult.size());
}

std::string ArduinoJsonJWT::encode(const char * cstr, int inputLen) {
    // prepare encoder
    base64_encodestate _state;
    base64_init_encodestate(&_state);

    // prepare buffer of correct length
    const auto bufferLength = static_cast<std::size_t>(base64_encode_expected_len(inputLen)) + 1;
    auto *     buffer       = new char[bufferLength];

    // encode to buffer
    int len = base64_encode_block(cstr, inputLen, &buffer[0], &_state);
    len += base64_encode_blockend(&buffer[len], &_state);
    buffer[len] = '\0';

    // convert to std::string, freeing buffer
    std::string result(buffer);
    delete[] buffer;
    buffer = nullptr;

    // remove padding
    while (!result.empty() && result.back() == '=') {
        result.pop_back();
    }

    // convert to URL safe form
    std::replace(result.begin(), result.end(), '+', '-');
    std::replace(result.begin(), result.end(), '/', '_');

    // return as string
    return result;
}

std::string ArduinoJsonJWT::decode(const std::string & value) {
    // convert to standard base64
    std::string modified_value = value;
    std::replace(modified_value.begin(), modified_value.end(), '-', '+');
    std::replace(modified_value.begin(), modified_value.end(), '_', '/');

    // prepare buffer of correct length
    const auto bufferLength = static_cast<std::size_t>(base64_decode_expected_len(modified_value.length()) + 1);
    auto *     buffer       = new char[bufferLength];

    // decode
    const int len = base64_decode_chars(modified_value.c_str(), static_cast<int>(modified_value.length()), &buffer[0]);
    buffer[len]   = '\0';

    // convert to std::string, freeing buffer
    std::string result(buffer);
    delete[] buffer;
    buffer = nullptr;

    // return as string
    return result;
}