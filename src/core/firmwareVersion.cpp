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

#include "firmwareVersion.h"

#include <cstdio>
#include <cstring>

namespace emsesp {

FirmwareVersion::FirmwareVersion(const std::string & s) {
    parse(s.c_str());
}

FirmwareVersion::FirmwareVersion(const char * s) {
    parse(s ? s : "");
}

int FirmwareVersion::major() const {
    return major_;
}

int FirmwareVersion::minor() const {
    return minor_;
}

int FirmwareVersion::patch() const {
    return patch_;
}

const std::string & FirmwareVersion::prerelease() const {
    return prerelease_;
}

bool operator<(const FirmwareVersion & a, const FirmwareVersion & b) {
    if (a.major_ != b.major_)
        return a.major_ < b.major_;
    if (a.minor_ != b.minor_)
        return a.minor_ < b.minor_;
    if (a.patch_ != b.patch_)
        return a.patch_ < b.patch_;
    return a.prerelease_ < b.prerelease_;
}

bool operator>(const FirmwareVersion & a, const FirmwareVersion & b) {
    return b < a;
}

bool operator==(const FirmwareVersion & a, const FirmwareVersion & b) {
    return a.major_ == b.major_ && a.minor_ == b.minor_ && a.patch_ == b.patch_ && a.prerelease_ == b.prerelease_;
}

bool operator!=(const FirmwareVersion & a, const FirmwareVersion & b) {
    return !(a == b);
}

bool operator>=(const FirmwareVersion & a, const FirmwareVersion & b) {
    return !(a < b);
}

bool operator<=(const FirmwareVersion & a, const FirmwareVersion & b) {
    return !(b < a);
}

void FirmwareVersion::parse(const char * s) {
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

} // namespace emsesp
