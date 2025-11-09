/*
 * EMS-ESP - https://github.com/emsesp/EMS-ESP
 * Copyright 2020-2025  emsesp.org - proddy, MichaelDvP
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

#ifndef EMSESP_COMMON_H
#define EMSESP_COMMON_H

#include <uuid/log.h>

using uuid::log::Level;

#if defined(EMSESP_DEBUG)
#define LOG_DEBUG(...) logger_.debug(__VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

#define LOG_INFO(...) logger_.info(__VA_ARGS__)
#define LOG_TRACE(...) logger_.trace(__VA_ARGS__)
#define LOG_NOTICE(...) logger_.notice(__VA_ARGS__)
#define LOG_WARNING(...) logger_.warning(__VA_ARGS__)
#define LOG_ERROR(...) logger_.err(__VA_ARGS__)

// String vector for handling lists of strings
using uuid::string_vector;

#ifdef FPSTR
#undef FPSTR
#endif

// clang-format off

// Flash string pointer (no-op for this platform)
#define FPSTR(pstr_pointer) pstr_pointer

// String literal macros for flash memory optimization
#define MAKE_WORD_CUSTOM(string_name, string_literal) static const char __pstr__##string_name[] = string_literal;
#define MAKE_WORD(string_name) MAKE_WORD_CUSTOM(string_name, #string_name)

// Accessors for strings and string lists
#define F_(string_name) (__pstr__##string_name)
#define FL_(list_name) (__pstr__L_##list_name)

// Translation macros - language settings must match system.cpp
#if defined(EMSESP_TEST)
// Test mode: use two languages (EN & DE) to save flash memory for tests
#define MAKE_WORD_TRANSLATION(list_name, en, de, ...)       static const char * const __pstr__L_##list_name[] = {en, de, nullptr};
#define MAKE_TRANSLATION(list_name, shortname, en, de, ...) static const char * const __pstr__L_##list_name[] = {shortname, en, de, nullptr};
#elif defined(EMSESP_EN_ONLY)
// English only mode
#define MAKE_WORD_TRANSLATION(list_name, en, ...)       static const char * const __pstr__L_##list_name[] = {en, nullptr};
#define MAKE_TRANSLATION(list_name, shortname, en, ...) static const char * const __pstr__L_##list_name[] = {shortname, en, nullptr};
#elif defined(EMSESP_DE_ONLY)
// English + German mode
#define MAKE_WORD_TRANSLATION(list_name, en, de, ...)       static const char * const __pstr__L_##list_name[] = {en, de, nullptr};
#define MAKE_TRANSLATION(list_name, shortname, en, de, ...) static const char * const __pstr__L_##list_name[] = {shortname, en, de, nullptr};
#else
// Full multilingual mode
#define MAKE_WORD_TRANSLATION(list_name, ...) static const char * const __pstr__L_##list_name[] = {__VA_ARGS__, nullptr};
#define MAKE_TRANSLATION(list_name, ...)      static const char * const __pstr__L_##list_name[] = {__VA_ARGS__, nullptr};
#endif

// String lists without translation support
#define MAKE_NOTRANSLATION(list_name, ...) static const char * const __pstr__L_##list_name[] = {__VA_ARGS__, nullptr};

// Enum macros
#define MAKE_ENUM_FIXED(enum_name, ...) static const char * const __pstr__L_##enum_name[] = {__VA_ARGS__, nullptr};
#define MAKE_ENUM(enum_name, ...)       static const char * const * __pstr__L_##enum_name[] = {__VA_ARGS__, nullptr};

// clang-format on

// load translations
#include "locale_translations.h"
#include "locale_common.h"

#endif
