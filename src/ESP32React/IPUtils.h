#ifndef IPUtils_h
#define IPUtils_h

#include <IPAddress.h>

class IPUtils {
  public:
    // Check if IP address is set (not INADDR_NONE)
    [[nodiscard]] static inline bool isSet(const IPAddress & ip) noexcept {
        return ip != INADDR_NONE;
    }
    
    // Check if IP address is not set (equals INADDR_NONE)
    [[nodiscard]] static inline bool isNotSet(const IPAddress & ip) noexcept {
        return ip == INADDR_NONE;
    }
};

#endif