/*
 * EMS-ESP - https://github.com/emsesp/EMS-ESP
 * Copyright 2020  Paul Derbyshire
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

#include "emsesp.h"

namespace emsesp {

using namespace std::placeholders; // for `_1` etc

WebDataService::WebDataService(AsyncWebServer * server, SecurityManager * securityManager)
    : _device_dataHandler(DEVICE_DATA_SERVICE_PATH,
                          securityManager->wrapCallback(std::bind(&WebDataService::device_data, this, _1, _2), AuthenticationPredicates::IS_AUTHENTICATED))
    , _excludeentities_dataHandler(EXCLUDE_ENTITIES_PATH,
                                   securityManager->wrapCallback(std::bind(&WebDataService::exclude_entities, this, _1, _2),
                                                                 AuthenticationPredicates::IS_AUTHENTICATED))
    , _deviceentities_dataHandler(DEVICE_ENTITIES_PATH,
                                  securityManager->wrapCallback(std::bind(&WebDataService::device_entities, this, _1, _2),
                                                                AuthenticationPredicates::IS_AUTHENTICATED))
    , _writevalue_dataHandler(WRITE_VALUE_SERVICE_PATH,
                              securityManager->wrapCallback(std::bind(&WebDataService::write_value, this, _1, _2), AuthenticationPredicates::IS_ADMIN))
    , _writesensor_dataHandler(WRITE_SENSOR_SERVICE_PATH,
                               securityManager->wrapCallback(std::bind(&WebDataService::write_sensor, this, _1, _2), AuthenticationPredicates::IS_ADMIN)) {
    server->on(CORE_DATA_SERVICE_PATH,
               HTTP_GET,
               securityManager->wrapRequest(std::bind(&WebDataService::core_data, this, _1), AuthenticationPredicates::IS_AUTHENTICATED));
    server->on(DEVICES_SERVICE_PATH,
               HTTP_GET,
               securityManager->wrapRequest(std::bind(&WebDataService::devices, this, _1), AuthenticationPredicates::IS_AUTHENTICATED));
    server->on(SCAN_DEVICES_SERVICE_PATH,
               HTTP_POST,
               securityManager->wrapRequest(std::bind(&WebDataService::scan_devices, this, _1), AuthenticationPredicates::IS_ADMIN));

    _device_dataHandler.setMethod(HTTP_POST);
    _device_dataHandler.setMaxContentLength(256);
    server->addHandler(&_device_dataHandler);

    _writevalue_dataHandler.setMethod(HTTP_POST);
    _writevalue_dataHandler.setMaxContentLength(256);
    server->addHandler(&_writevalue_dataHandler);

    _writesensor_dataHandler.setMethod(HTTP_POST);
    _writesensor_dataHandler.setMaxContentLength(256);
    server->addHandler(&_writesensor_dataHandler);

    _excludeentities_dataHandler.setMethod(HTTP_POST);
    _excludeentities_dataHandler.setMaxContentLength(256);
    server->addHandler(&_excludeentities_dataHandler);

    _deviceentities_dataHandler.setMethod(HTTP_POST);
    _deviceentities_dataHandler.setMaxContentLength(256);
    server->addHandler(&_deviceentities_dataHandler);
}

void WebDataService::scan_devices(AsyncWebServerRequest * request) {
    EMSESP::scan_devices();
    request->send(200);
}

void WebDataService::devices(AsyncWebServerRequest * request) {
    AsyncJsonResponse * response = new AsyncJsonResponse(false, EMSESP_JSON_SIZE_LARGE_DYN);
    JsonObject          root     = response->getRoot();

    JsonArray devices = root.createNestedArray("devices");
    for (auto & emsdevice : EMSESP::emsdevices) {
        if (emsdevice->has_entities()) {
            JsonObject obj = devices.createNestedObject();
            obj["i"]       = emsdevice->unique_id(); // a unique id

            // shortname - we prefix the count to make it unique
            uint8_t device_index = EMSESP::device_index(emsdevice->device_type(), emsdevice->unique_id());
            if (device_index) {
                char s[10];
                obj["s"] = emsdevice->device_type_name() + Helpers::smallitoa(s, device_index);
            } else {
                obj["s"] = emsdevice->device_type_name();
            }
        }
    }

    response->setLength();
    request->send(response);
}

void WebDataService::core_data(AsyncWebServerRequest * request) {
    AsyncJsonResponse * response = new AsyncJsonResponse(false, EMSESP_JSON_SIZE_XLARGE_DYN);
    JsonObject          root     = response->getRoot();

    // devices
    JsonArray devices = root.createNestedArray("devices");
    for (auto & emsdevice : EMSESP::emsdevices) {
        if (emsdevice) {
            JsonObject obj = devices.createNestedObject();
            obj["i"]       = emsdevice->unique_id();        // a unique id
            obj["t"]       = emsdevice->device_type_name(); // type
            obj["b"]       = emsdevice->brand_to_string();  // brand

            // shortname - we prefix the count to make it unique
            uint8_t device_index = EMSESP::device_index(emsdevice->device_type(), emsdevice->unique_id());
            if (device_index) {
                char s[10];
                obj["s"] = emsdevice->device_type_name() + Helpers::smallitoa(s, device_index);
            } else {
                obj["s"] = emsdevice->device_type_name();
            }

            obj["n"] = emsdevice->name();           // name
            obj["d"] = emsdevice->device_id();      // deviceid
            obj["p"] = emsdevice->product_id();     // productid
            obj["v"] = emsdevice->version();        // version
            obj["e"] = emsdevice->count_entities(); // number of entities (device values)
        }
    }

    // always create sensors, even if it's empty
    JsonArray sensors = root.createNestedArray("sensors");
    if (EMSESP::have_sensors()) {
        for (const auto & sensor : EMSESP::dallassensor_.sensors()) {
            JsonObject obj = sensors.createNestedObject();
            obj["is"]      = sensor.id_str(); // id
            obj["n"]       = sensor.name();   // name

            // only send temp if it has a value
            if (Helpers::hasValue(sensor.temperature_c)) {
                obj["t"] = (float)(sensor.temperature_c) / 10; // temperature
            }

            obj["o"] = (float)(sensor.offset()) / 10; // offset
        }
    }

    if (EMSESP::system_.analog_enabled()) {
        root["analog"] = EMSESP::system_.analog();
    }

    response->setLength();
    request->send(response);
}

// The unique_id is the unique record ID from the Web table to identify which device to load
// Compresses the JSON using MsgPack https://msgpack.org/index.html
void WebDataService::device_data(AsyncWebServerRequest * request, JsonVariant & json) {
    if (json.is<JsonObject>()) {
        MsgpackAsyncJsonResponse * response = new MsgpackAsyncJsonResponse(false, EMSESP_JSON_SIZE_XXLARGE_DYN);
        for (const auto & emsdevice : EMSESP::emsdevices) {
            if (emsdevice) {
                if (emsdevice->unique_id() == json["id"]) {
                    // wait max 2.5 sec for updated data (post_send_delay is 2 sec)
                    for (uint16_t i = 0; i < (emsesp::TxService::POST_SEND_DELAY + 500) && EMSESP::wait_validate(); i++) {
                        delay(1);
                    }
                    EMSESP::wait_validate(0); // reset in case of timeout
#ifndef EMSESP_STANDALONE
                    JsonObject output = response->getRoot();
                    emsdevice->generate_values_web(output);
#endif
                    response->setLength();
                    request->send(response);
                    return;
                }
            }
        }
    }

    // invalid but send ok
    AsyncWebServerResponse * response = request->beginResponse(200);
    request->send(response);
}

// send back filtered list of device entities
void WebDataService::device_entities(AsyncWebServerRequest * request, JsonVariant & json) {
    if (json.is<JsonObject>()) {
        MsgpackAsyncJsonResponse * response = new MsgpackAsyncJsonResponse(true, EMSESP_JSON_SIZE_XXLARGE_DYN);
        for (const auto & emsdevice : EMSESP::emsdevices) {
            if (emsdevice) {
                if (emsdevice->unique_id() == json["id"]) {
#ifndef EMSESP_STANDALONE
                    JsonArray output = response->getRoot();
                    emsdevice->generate_values_web_all(output);
#endif
                    response->setLength();
                    request->send(response);
                    return;
                }
            }
        }
    }

    // invalid, but send OK anyway
    AsyncWebServerResponse * response = request->beginResponse(200);
    request->send(response);
}

// takes a list of excluded shortnames from the webUI
// saves it in the customization service
// and updates the entity list real-time
void WebDataService::exclude_entities(AsyncWebServerRequest * request, JsonVariant & json) {
    if (json.is<JsonObject>()) {
        // find the device using the unique_id
        for (const auto & emsdevice : EMSESP::emsdevices) {
            if (emsdevice) {
                uint8_t unique_device_id = json["id"];
                if (emsdevice->unique_id() == unique_device_id) {
                    JsonArray entity_ids = json["entity_ids"];

                    std::vector<uint8_t> temp;
                    for (JsonVariant id : entity_ids) {
                        uint8_t entity_id = id.as<int>();
                        emsdevice->exclude_entity(entity_id); // this will have immediate affect
                        temp.push_back(entity_id);
                    }

                    // Save the list to the customization file
                    EMSESP::webCustomizationService.update(
                        [&](WebCustomization & settings) {
                            // if it exists overwrite it
                            for (auto & entityCustomization : settings.entityCustomizations) {
                                if (entityCustomization.id == unique_device_id) {
                                    entityCustomization.entity_ids.clear();
                                    for (uint8_t i = 0; i < temp.size(); i++) {
                                        entityCustomization.entity_ids.push_back(temp[i]);
                                        settings.entityCustomizations.push_back(entityCustomization);
                                    }
                                    return StateUpdateResult::CHANGED;
                                }
                            }
                            // create a new entry in the list
                            EntityCustomization new_entry;
                            new_entry.id = unique_device_id;
                            for (uint8_t i = 0; i < temp.size(); i++) {
                                new_entry.entity_ids.push_back(temp[i]);
                            }
                            settings.entityCustomizations.push_back(new_entry);
                            return StateUpdateResult::CHANGED;
                        },
                        "local");

                    break;
                }
            }
        }
    }

    AsyncWebServerResponse * response = request->beginResponse(200); // OK
    request->send(response);
}

// takes a command and its data value from a specific EMS Device, from the Web
// assumes the service has been checked for admin authentication
void WebDataService::write_value(AsyncWebServerRequest * request, JsonVariant & json) {
    if (json.is<JsonObject>()) {
        JsonObject dv        = json["devicevalue"];
        uint8_t    unique_id = json["id"];

        // using the unique ID from the web find the real device type
        // id is the selected device
        for (const auto & emsdevice : EMSESP::emsdevices) {
            if (emsdevice) {
                if (emsdevice->unique_id() == unique_id) {
                    // parse the command as it could have a hc or wwc prefixed, e.g. hc2/seltemp
                    const char * cmd = dv["c"];                                // the command
                    int8_t       id  = -1;                                     // default
                    cmd              = Command::parse_command_string(cmd, id); // extract hc or wwc

                    // create JSON for output
                    AsyncJsonResponse * response = new AsyncJsonResponse(false, EMSESP_JSON_SIZE_SMALL);
                    JsonObject          output   = response->getRoot();

                    // the data could be in any format, but we need string
                    // authenticated is always true
                    JsonVariant data        = dv["v"]; // the value in any format
                    uint8_t     return_code = CommandRet::OK;
                    uint8_t     device_type = emsdevice->device_type();
                    if (data.is<const char *>()) {
                        return_code = Command::call(device_type, cmd, data.as<const char *>(), true, id, output);
                    } else if (data.is<int>()) {
                        char s[10];
                        return_code = Command::call(device_type, cmd, Helpers::render_value(s, data.as<int16_t>(), 0), true, id, output);
                    } else if (data.is<float>()) {
                        char s[10];
                        return_code = Command::call(device_type, cmd, Helpers::render_value(s, (float)data.as<float>(), 1), true, id, output);
                    } else if (data.is<bool>()) {
                        return_code = Command::call(device_type, cmd, data.as<bool>() ? "true" : "false", true, id, output);
                    }

                    // write debug
                    if (return_code != CommandRet::OK) {
                        EMSESP::logger().err(F("Write command failed %s (%s)"), (const char *)output["message"], Command::return_code_string(return_code).c_str());
                    } else {
                        EMSESP::logger().debug(F("Write command successful"));
                    }

                    response->setCode((return_code == CommandRet::OK) ? 200 : 204);
                    response->setLength();
                    request->send(response);
                    return;
                }
            }
        }
    }

    AsyncWebServerResponse * response = request->beginResponse(204); // Write command failed
    request->send(response);
}

// takes a sensor name and optional offset from the WebUI and update the customization settings
// via the Dallas service
void WebDataService::write_sensor(AsyncWebServerRequest * request, JsonVariant & json) {
    bool ok = false;
    if (json.is<JsonObject>()) {
        JsonObject sensor = json;

        std::string id_str = sensor["id_str"]; // this is the key
        std::string name   = sensor["name"];

        // calculate offset. We'll convert it to an int and * 10
        float   offset   = sensor["offset"];
        int16_t offset10 = offset * 10;

        ok = EMSESP::dallassensor_.update(id_str, name, offset10);
    }

    AsyncWebServerResponse * response = request->beginResponse(ok ? 200 : 204);
    request->send(response);
}

} // namespace emsesp