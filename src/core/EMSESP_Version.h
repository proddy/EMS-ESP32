/*
 * EMS-ESP - https://github.com/emsesp/EMS-ESP
 * Copyright 2020-2026  emsesp.org
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EMSESP_Version_H
#define EMSESP_Version_H

#include <cstdio>
#include <cstring>
#include <string>

// Drop-in lightweight replacement for the subset of the semver library actually used by EMS-ESP.
// The previous semver library (lib/semver) builds a std::map + std::function-based state machine on
// every parse, which fragments the internal heap on the ESP32. This replacement does no heap
// allocation beyond the std::string member for the prerelease tag, and matches the API surface
// we consume: construction from string, major()/minor()/patch()/prerelease(), and operator>/</==.
//
// Only strict numeric precedence (major.minor.patch) is used for comparison in EMS-ESP, so we
// intentionally ignore prerelease tags during comparison rather than implement the full semver
// ordering rules. This is consistent with how the old code was used (callers only check major/
// minor/patch numerically; prerelease() is only read for logging).

namespace version {

class EMSESP_Version {
  public:
    EMSESP_Version() = default;

    // Construct from a version string like "3.9.0-dev.14" or "3.9.0".
    // Anything past a '-' or '+' is kept as the prerelease string and not interpreted.
    explicit EMSESP_Version(const std::string & s) {
        parse(s.c_str());
    }

    explicit EMSESP_Version(const char * s) {
        parse(s ? s : "");
    }

    int major() const {
        return major_;
    }
    int minor() const {
        return minor_;
    }
    int patch() const {
        return patch_;
    }
    const std::string & prerelease() const {
        return prerelease_;
    }

    // Numeric-only comparison (major.minor.patch). Prerelease tags are ignored on purpose.
    friend bool operator<(const EMSESP_Version & a, const EMSESP_Version & b) {
        if (a.major_ != b.major_)
            return a.major_ < b.major_;
        if (a.minor_ != b.minor_)
            return a.minor_ < b.minor_;
        if (a.patch_ != b.patch_)
            return a.patch_ < b.patch_;
        return a.prerelease_ < b.prerelease_;
    }

    friend bool operator>(const EMSESP_Version & a, const EMSESP_Version & b) {
        return b < a;
    }

    friend bool operator==(const EMSESP_Version & a, const EMSESP_Version & b) {
        return a.major_ == b.major_ && a.minor_ == b.minor_ && a.patch_ == b.patch_ && a.prerelease_ == b.prerelease_;
    }

    friend bool operator!=(const EMSESP_Version & a, const EMSESP_Version & b) {
        return !(a == b);
    }

    friend bool operator>=(const EMSESP_Version & a, const EMSESP_Version & b) {
        return !(a < b);
    }

    friend bool operator<=(const EMSESP_Version & a, const EMSESP_Version & b) {
        return !(b < a);
    }

  private:
    int         major_ = 0;
    int         minor_ = 0;
    int         patch_ = 0;
    std::string prerelease_;

    void parse(const char * s) {
        major_ = minor_ = patch_ = 0;
        prerelease_.clear();
        if (s == nullptr || *s == '\0') {
            return;
        }
        // parse numeric major.minor.patch; accept partial ("3", "3.9", "3.9.0")
        sscanf(s, "%d.%d.%d", &major_, &minor_, &patch_);
        // capture prerelease tag after '-' if present (stop at '+' which is build metadata)
        const char * dash = strchr(s, '-');
        if (dash != nullptr) {
            const char * plus = strchr(dash, '+');
            if (plus != nullptr) {
                prerelease_.assign(dash + 1, plus - dash - 1);
            } else {
                prerelease_.assign(dash + 1);
            }
        }
    }
};

} // namespace version

#endif
