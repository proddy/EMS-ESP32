/*
 * EMS-ESP - https://github.com/emsesp/EMS-ESP
 * Copyright 2020-2025  emsesp.org
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
#include "WebSchedulerService.h"

#include "shuntingYard.h"

namespace emsesp {

WebSchedulerService::WebSchedulerService(AsyncWebServer * server, FS * fs, SecurityManager * securityManager)
    : _httpEndpoint(WebScheduler::read, WebScheduler::update, this, server, EMSESP_SCHEDULER_SERVICE_PATH, securityManager, AuthenticationPredicates::IS_AUTHENTICATED)
    , _fsPersistence(WebScheduler::read, WebScheduler::update, this, fs, EMSESP_SCHEDULER_FILE) {
}

// load the settings when the service starts
void WebSchedulerService::begin() {
    _fsPersistence.readFromFS();

    // save a local pointer to the scheduler item list
    EMSESP::webSchedulerService.read([&](WebScheduler & webScheduler) { scheduleItems_ = &webScheduler.scheduleItems; });

    EMSESP::logger().info("Starting Scheduler service");
    char topic[Mqtt::MQTT_TOPIC_MAX_SIZE];
    snprintf(topic, sizeof(topic), "%s/#", F_(scheduler));
    Mqtt::subscribe(EMSdevice::DeviceType::SCHEDULER, topic, nullptr); // use empty function callback
#ifndef EMSESP_STANDALONE
    if (EMSESP::system_.PSram()) {
#if defined(CONFIG_FREERTOS_UNICORE) || (EMSESP_SCHEDULER_RUNNING_CORE < 0)
        xTaskCreate((TaskFunction_t)scheduler_task, "scheduler_task", EMSESP_SCHEDULER_STACKSIZE, NULL, EMSESP_SCHEDULER_PRIORITY, NULL);
#else
        xTaskCreatePinnedToCore(
            (TaskFunction_t)scheduler_task, "scheduler_task", EMSESP_SCHEDULER_STACKSIZE, NULL, EMSESP_SCHEDULER_PRIORITY, NULL, EMSESP_SCHEDULER_RUNNING_CORE);
#endif
    }
#endif
}

// this creates the scheduler file, saving it to the FS
// and also calls when the Scheduler web page is refreshed
void WebScheduler::read(WebScheduler & webScheduler, JsonObject root) {
    JsonArray schedule = root["schedule"].to<JsonArray>();
    uint8_t   counter  = 1;
    for (const ScheduleItem & scheduleItem : webScheduler.scheduleItems) {
        JsonObject si = schedule.add<JsonObject>();
        si["id"]      = counter++; // id is only used to render the table and must be unique. 0 is for Dashboard
        si["flags"]   = scheduleItem.flags;
        si["active"]  = scheduleItem.flags != SCHEDULEFLAG_SCHEDULE_IMMEDIATE ? scheduleItem.active : false;
        si["time"]    = scheduleItem.flags != SCHEDULEFLAG_SCHEDULE_IMMEDIATE ? scheduleItem.time : "";
        si["cmd"]     = scheduleItem.cmd;
        si["value"]   = scheduleItem.value;
        si["name"]    = (const char *)scheduleItem.name;
    }
}

// call on initialization and also when the Schedule web page is saved
// this loads the data into the internal class
StateUpdateResult WebScheduler::update(JsonObject root, WebScheduler & webScheduler) {
    // reset the list
    Command::erase_device_commands(EMSdevice::DeviceType::SCHEDULER);
    for (ScheduleItem & scheduleItem : webScheduler.scheduleItems) {
        char key[sizeof(scheduleItem.name) + 2];
        snprintf(key, sizeof(key), "s:%s", scheduleItem.name);
        EMSESP::nvs_.remove(key);
    }
    webScheduler.scheduleItems.clear();
    EMSESP::webSchedulerService.ha_reset();

    // build up the list of schedule items
    auto scheduleItems = root["schedule"].as<JsonArray>();
    for (const JsonObject schedule : scheduleItems) {
        // create each schedule item, overwriting any previous settings
        // ignore the id (as this is only used in the web for table rendering)
        auto si   = ScheduleItem();
        si.active = schedule["active"];
        si.flags  = schedule["flags"];
        si.time   = si.flags == SCHEDULEFLAG_SCHEDULE_IMMEDIATE ? "" : schedule["time"].as<std::string>();
        si.cmd    = schedule["cmd"].as<std::string>();
        si.value  = schedule["value"].as<std::string>();
        strlcpy(si.name, schedule["name"].as<const char *>(), sizeof(si.name));

        // calculated elapsed minutes
        si.elapsed_min = Helpers::string2minutes(si.time.c_str());
        si.retry_cnt   = 0xFF; // no startup retries

        webScheduler.scheduleItems.push_back(si); // add to list
        if (webScheduler.scheduleItems.back().name[0] != '\0') {
            char key[sizeof(webScheduler.scheduleItems.back().name) + 2];
            snprintf(key, sizeof(key), "s:%s", webScheduler.scheduleItems.back().name);
            if (EMSESP::nvs_.isKey(key) && webScheduler.scheduleItems.back().flags != SCHEDULEFLAG_SCHEDULE_IMMEDIATE) {
                webScheduler.scheduleItems.back().active = EMSESP::nvs_.getBool(key);
            }
            Command::add(
                EMSdevice::DeviceType::SCHEDULER,
                webScheduler.scheduleItems.back().name,
                [webScheduler](const char * value, const int8_t id) {
                    return EMSESP::webSchedulerService.command_setvalue(value, id, webScheduler.scheduleItems.back().name);
                },
                FL_(schedule_cmd),
                CommandFlag::ADMIN_ONLY);
        }
    }
    return StateUpdateResult::CHANGED;
}

// set active by api command
bool WebSchedulerService::command_setvalue(const char * value, const int8_t id, const char * name) {
    bool v;
    if (!Helpers::value2bool(value, v)) {
        return false;
    }

    for (ScheduleItem & scheduleItem : *scheduleItems_) {
        if (Helpers::toLower(scheduleItem.name) == Helpers::toLower(name)) {
            if (scheduleItem.active == v) {
                return true;
            }

            scheduleItem.active = v;
            publish_single(name, v);

            if (EMSESP::mqtt_.get_publish_onchange(0)) {
                publish();
            }
            // save new state to nvs #2946
            if (scheduleItem.flags != SCHEDULEFLAG_SCHEDULE_IMMEDIATE) {
                char key[sizeof(scheduleItem.name) + 2];
                snprintf(key, sizeof(key), "s:%s", scheduleItem.name);
                EMSESP::nvs_.putBool(key, scheduleItem.active);
            }
            return true;
        }
    }
    return false;
}

// process json output for info/commands and value_info
bool WebSchedulerService::get_value_info(JsonObject output, const char * cmd) {
    if (scheduleItems_->empty()) {
        return true;
    }

    if (!strlen(cmd) || !strcmp(cmd, F_(values)) || !strcmp(cmd, F_(info))) {
        // list all names
        for (const ScheduleItem & scheduleItem : *scheduleItems_) {
            if (scheduleItem.name[0] != '\0') {
                Mqtt::add_value_bool(output, scheduleItem.name, scheduleItem.active);
            }
        }
        return true;
    }

    if (!strcmp(cmd, F_(entities))) {
        uint8_t i = 0;
        char    name[20];
        for (const ScheduleItem & scheduleItem : *scheduleItems_) {
            strlcpy(name, scheduleItem.name[0] == '\0' ? Helpers::smallitoa(name, i++) : scheduleItem.name, sizeof(name));
            get_value_json(output[name].to<JsonObject>(), scheduleItem);
        }
        return true;
    }

    if (!strcmp(cmd, F_(metrics))) {
        std::string metrics = get_metrics_prometheus();
        if (!metrics.empty()) {
            output["api_data"] = metrics;
            return true;
        }
        return false;
    }

    const char * attribute_s = Command::get_attribute(cmd);
    for (const ScheduleItem & scheduleItem : *scheduleItems_) {
        if (Helpers::toLower(scheduleItem.name) == cmd) {
            get_value_json(output, scheduleItem);
            return Command::get_attribute(output, cmd, attribute_s);
        }
    }

    return false; // not found
}

// generate Prometheus metrics format from scheduler values
std::string WebSchedulerService::get_metrics_prometheus() {
    std::string result;
    result.reserve(scheduleItems_->size() * 140);
    for (const ScheduleItem & scheduleItem : *scheduleItems_) {
        if (scheduleItem.name[0] == '\0') {
            continue;
        }
        result += (std::string) "# HELP emsesp_" + scheduleItem.name + " " + scheduleItem.name + ", boolean, readable, visible, writable\n";
        result += (std::string) "# TYPE emsesp_" + scheduleItem.name + " gauge\n";
        result += (std::string) "emsesp_" + scheduleItem.name + " " + (scheduleItem.active ? "1\n" : "0\n");
    }
    return result;
}

// build the json for specific entity
void WebSchedulerService::get_value_json(JsonObject output, const ScheduleItem & scheduleItem) {
    output["name"]     = (const char *)scheduleItem.name;
    output["fullname"] = (const char *)scheduleItem.name;
    output["type"]     = "boolean";
    Mqtt::add_value_bool(output, "value", scheduleItem.active);
    if (scheduleItem.flags == SCHEDULEFLAG_SCHEDULE_CONDITION) {
        output["condition"] = scheduleItem.time;
    } else if (scheduleItem.flags == SCHEDULEFLAG_SCHEDULE_ONCHANGE) {
        output["onchange"] = scheduleItem.time;
    } else if (scheduleItem.flags == SCHEDULEFLAG_SCHEDULE_TIMER) {
        output["timer"] = scheduleItem.time;
    } else if (scheduleItem.flags != SCHEDULEFLAG_SCHEDULE_IMMEDIATE) {
        output["time"] = scheduleItem.time;
    }
    output["command"]   = scheduleItem.cmd;
    output["cmd_data"]  = scheduleItem.value;
    bool hasName        = scheduleItem.name[0] != '\0';
    output["readable"]  = hasName;
    output["writeable"] = hasName;
    output["visible"]   = hasName;
}

// publish single value
void WebSchedulerService::publish_single(const char * name, const bool state) {
    if (!Mqtt::enabled() || !Mqtt::publish_single() || name == nullptr || name[0] == '\0') {
        return;
    }

    char topic[Mqtt::MQTT_TOPIC_MAX_SIZE];
    if (Mqtt::publish_single2cmd()) {
        snprintf(topic, sizeof(topic), "%s/%s", F_(scheduler), name);
    } else {
        snprintf(topic, sizeof(topic), "%s_data/%s", F_(scheduler), name);
    }

    char payload[12];
    Mqtt::queue_publish(topic, Helpers::render_boolean(payload, state));
}

// publish to Mqtt
void WebSchedulerService::publish(const bool force) {
    if (!Mqtt::enabled() || scheduleItems_->empty()) {
        return;
    }
    if (force) {
        if (Mqtt::publish_single()) {
            for (const ScheduleItem & scheduleItem : *scheduleItems_) {
                publish_single(scheduleItem.name, scheduleItem.active);
            }
            return;
        } else if (!EMSESP::mqtt_.get_publish_onchange(0)) {
            return; // wait for first time period
        }
    }

    JsonDocument doc(PSRAM_DOC);
    JsonObject   output     = doc.to<JsonObject>();
    bool         ha_created = ha_configdone_;
    for (const ScheduleItem & scheduleItem : *scheduleItems_) {
        if (scheduleItem.name[0] != '\0' && !output[scheduleItem.name].is<JsonVariantConst>()) {
            Mqtt::add_value_bool(output, (const char *)scheduleItem.name, scheduleItem.active);

            // create HA config
            if (Mqtt::ha_enabled() && !ha_configdone_) {
                JsonDocument config(PSRAM_DOC);
                config["~"] = Mqtt::base();

                char stat_t[50];
                snprintf(stat_t, sizeof(stat_t), "~/%s_data", F_(scheduler));
                config["stat_t"] = stat_t;

                char val_obj[50];
                char val_cond[65];
                snprintf(val_obj, sizeof(val_obj), "value_json['%s']", scheduleItem.name);
                snprintf(val_cond, sizeof(val_cond), "%s is defined", val_obj);

                // Optimized: use stack buffer instead of string concatenation to avoid heap allocations
                char val_tpl[150];
                if (Mqtt::discovery_type() == Mqtt::discoveryType::HOMEASSISTANT) {
                    snprintf(val_tpl, sizeof(val_tpl), "{{%s if %s}}", val_obj, val_cond);
                } else {
                    snprintf(val_tpl, sizeof(val_tpl), "{{%s}}", val_obj); // omit value conditional Jinja2 template code
                }
                config["val_tpl"] = val_tpl;

                char uniq_s[70];
                snprintf(uniq_s, sizeof(uniq_s), "%s_%s", F_(scheduler), scheduleItem.name);

                config["uniq_id"] = uniq_s;
                config["name"]    = (const char *)scheduleItem.name;
                // Optimized: use stack buffer instead of string concatenation
                char def_ent_id[80];
                snprintf(def_ent_id, sizeof(def_ent_id), "switch.%s", uniq_s);
                config["def_ent_id"] = def_ent_id;

                char topic[Mqtt::MQTT_TOPIC_MAX_SIZE];
                char command_topic[Mqtt::MQTT_TOPIC_MAX_SIZE];

                snprintf(topic, sizeof(topic), "switch/%s/%s_%s/config", Mqtt::basename().c_str(), F_(scheduler), scheduleItem.name);
                snprintf(command_topic, sizeof(command_topic), "~/%s/%s", F_(scheduler), scheduleItem.name);
                config["cmd_t"] = command_topic;

                Mqtt::add_ha_bool(config.as<JsonObject>());
                Mqtt::add_ha_dev_section(config.as<JsonObject>(), F_(scheduler), !ha_created);
                Mqtt::add_ha_avty_section(config.as<JsonObject>(), stat_t, val_cond);

                ha_created |= Mqtt::queue_ha(topic, config.as<JsonObject>());
            }
        }
    }

    ha_configdone_ = ha_created;

    if (!doc.isNull()) {
        char topic[Mqtt::MQTT_TOPIC_MAX_SIZE];
        snprintf(topic, sizeof(topic), "%s_data", F_(scheduler));
        Mqtt::queue_publish(topic, output);
    }
}

// count number of entries, default: only named items
uint8_t WebSchedulerService::count_entities(bool cmd_only) {
    uint8_t count = 0;
    for (const ScheduleItem & scheduleItem : *scheduleItems_) {
        if (scheduleItem.name[0] != '\0' || !cmd_only) {
            count++;
        }
    }
    return count;
}

// execute scheduled command
bool WebSchedulerService::command(const char * name, const std::string & command, const std::string & data) {
    std::string cmd = Helpers::toLower(command);

    // check http commands. e.g.
    // tasmota(get): http://<tasmotaIP>/cm?cmnd=power%20ON
    // shelly(get): http://<shellyIP>/relais/0?turn=on
    // parse json
    JsonDocument doc;
    if (deserializeJson(doc, cmd) == DeserializationError::Ok) {
        std::string url = doc["url"] | "";
        // for a GET with parameters replace commands with values
        // don't search the complete url, it may contain a devicename in path
        auto q = url.find_first_of('?');
        if (q != std::string::npos) {
            auto s = url.substr(q + 1); // copy only parameters
            auto l = s.length();
            commands(s, false);
            url.replace(q + 1, l, s);
        }
        std::string value  = doc["value"] | data;   // extract value if its in the command, or take the data
        std::string method = doc["method"] | "GET"; // default GET
        commands(value, false);
        if (value.length()) {
            method = "POST";
        }
        std::string result;
        int         httpResult = 0;
#ifndef NO_TLS_SUPPORT
        if (Helpers::toLower(url.c_str()).starts_with("https://")) {
            WiFiClient *    basic_client = new WiFiClient;
            ESP_SSLClient * ssl_client   = new ESP_SSLClient;
            ssl_client->setInsecure(); // with root CA we should set here: ssl_client->setCACert(rootCACert);
            ssl_client->setBufferSizes(1024, 1024);
            ssl_client->setSessionTimeout(120); // Set the timeout in seconds (>=120 seconds)
            url.replace(0, 8, "");
            std::string host  = url;
            auto        index = url.find_first_of('/');
            if (index != std::string::npos) {
                host = url.substr(0, index);
                url.replace(0, index, "");
            }
            // EMSESP::logger().debug("Host: %s, URL: %s", host.c_str(), url.c_str());
            ssl_client->setClient(basic_client);
            if (ssl_client->connect(host.c_str(), 443)) {
                if (value.length() || Helpers::toLower(method) == "post") {
                    // EMSESP::logger().debug("POST %s HTTP/1.1", url.c_str());
                    ssl_client->print("POST ");
                    ssl_client->print(url.c_str());
                    ssl_client->println(" HTTP/1.1");
                    ssl_client->print("Host: ");
                    ssl_client->println(host.c_str());
                    bool content_set = false;
                    for (JsonPair p : doc["header"].as<JsonObject>()) {
                        content_set |= (emsesp::Helpers::toLower(p.key().c_str()) == "content-type");
                        ssl_client->print(p.key().c_str());
                        ssl_client->print(": ");
                        ssl_client->println(p.value().as<std::string>().c_str());
                    }
                    if (!content_set) {
                        ssl_client->print("Content-Type: ");
                        if (value.starts_with('{')) {
                            ssl_client->println(asyncsrv::T_application_json);
                        } else {
                            ssl_client->println(asyncsrv::T_text_plain);
                        }
                    }
                    ssl_client->print("Content-Length: ");
                    ssl_client->println(value.length());
                    ssl_client->println("Connection: close");
                    ssl_client->print("\r\n");
                    ssl_client->print(value.c_str());
                } else {
                    // EMSESP::logger().debug("GET %s HTTP/1.1", url.c_str());
                    ssl_client->print("GET ");
                    ssl_client->print(url.c_str());
                    ssl_client->println(" HTTP/1.1");
                    ssl_client->print("Host: ");
                    ssl_client->println(host.c_str());
                    for (JsonPair p : doc["header"].as<JsonObject>()) {
                        ssl_client->print(p.key().c_str());
                        ssl_client->print(": ");
                        ssl_client->println(p.value().as<std::string>().c_str());
                    }
                    ssl_client->println("Connection: close");
                }
                auto ms = millis();
                while (ssl_client->connected() && !ssl_client->available() && millis() - ms < 3000) {
                    delay(0);
                }
                while (ssl_client->available()) {
                    result += (char)ssl_client->read();
                }
                ssl_client->stop();
                // EMSESP::logger().debug("HTTPS response: %s", result.c_str());
                index = result.find_first_of(' ');
                if (index != std::string::npos) {
                    httpResult = stoi(result.substr(index + 1, 3));
                    // EMSESP::logger().debug("HTTPS code: %i", httpResult);
                }
                index = result.find("\r\n\r\n");
                if (index != std::string::npos) {
                    result.replace(0, index + 4, "");
                    // EMSESP::logger().debug("HTTPS response: %s", result.c_str());
                }
            } else {
                EMSESP::logger().warning("HTTPS connection failed");
            }
            delete ssl_client;
            delete basic_client;
            // check HTTP return code
            if (httpResult != 200) {
                EMSESP::logger().warning("Schedule '%s': URL command failed with http code %d", name, httpResult);
                return false;
            }
            return true;
        } else
#endif
            if (Helpers::toLower(url.c_str()).starts_with("http://")) {
            HTTPClient * http = new HTTPClient;
            if (http->begin(url.c_str())) {
                bool content_set = false;
                for (JsonPair p : doc["header"].as<JsonObject>()) {
                    http->addHeader(p.key().c_str(), p.value().as<std::string>().c_str());
                    content_set |= p.key() == "content-type";
                }
                // if there is data, force a POST
                if (Helpers::toLower(method) == "post") { // we have all lowercase
                    if (!content_set) {
                        // http->addHeader("Content-Type", value.find_first_of('{') != std::string::npos ? "application/json" : "text/plain");
                        if (value.starts_with('{')) {
                            http->addHeader(asyncsrv::T_Content_Type, asyncsrv::T_application_json, false); // auto-set to JSON
                        } else {
                            http->addHeader(asyncsrv::T_Content_Type, asyncsrv::T_text_plain, false); // auto-set to JSON
                        }
                    }
                    httpResult = http->POST(value.c_str());
                } else {
                    httpResult = http->GET(); // normal GET
                    if (httpResult > 0) {
                        result = http->getString().c_str();
                    }
                }
            }
            http->end();
            delete http;
            // check HTTP return code
            if (httpResult != 200) {
                EMSESP::logger().warning("Schedule '%s': URL command failed with http code %d", name, httpResult);
                return false;
            }
#if defined(EMSESP_DEBUG)
            EMSESP::logger().debug("Schedule %s: URL '%s' command successful with http code %d", name, url.c_str(), httpResult);
#endif
            return true;
        }
        // we can add other json tests here
    }

    doc.clear();
    JsonObject input = doc.to<JsonObject>();
    if (!data.empty()) { // empty data queries a value
        input["data"] = data;
    }

    JsonDocument doc_output; // only for commands without output
    JsonObject   output = doc_output.to<JsonObject>();

    // prefix "api/" to command string
    char command_str[COMMAND_MAX_LENGTH];
    snprintf(command_str, sizeof(command_str), "/api/%s", cmd.c_str());

    uint8_t return_code = Command::process(command_str, true, input, output); // admin set
    if (return_code == CommandRet::OK) {
#if defined(EMSESP_DEBUG)
        EMSESP::logger().debug("Schedule command '%s' with data '%s' was successful", cmd.c_str(), data.c_str());
#endif
        if (data.empty() && output.size()) {
            Mqtt::queue_publish("response", output);
        }
        return true;
    }

    char error[100];
    if (output.size()) {
        // check for empty name
        snprintf(error, sizeof(error), "Schedule %s: %s", name ? name : "", (const char *)output["message"]); // use error message if we have it
    } else {
        snprintf(error, sizeof(error), "Schedule %s: command %s failed with error %s", name, cmd.c_str(), Command::return_code_string(return_code));
    }

    EMSESP::logger().warning(error);
    return false;
}

// called from emsesp.cpp on every entity-change
// queue schedules to be handled executed in scheduler-loop
bool WebSchedulerService::onChange(const char * cmd) {
    for (ScheduleItem & scheduleItem : *scheduleItems_) {
        if (scheduleItem.active && scheduleItem.flags == SCHEDULEFLAG_SCHEDULE_ONCHANGE
            && Helpers::toLower(scheduleItem.time.c_str()).find(Helpers::toLower(cmd)) != std::string::npos) {
            cmd_changed_.push_back(&scheduleItem);
            return true;
        }
    }
    return false;
}

// handle condition schedules, parse string stored in schedule.time field
void WebSchedulerService::condition() {
    for (ScheduleItem & scheduleItem : *scheduleItems_) {
        if (scheduleItem.active && scheduleItem.flags == SCHEDULEFLAG_SCHEDULE_CONDITION) {
            auto match = compute(scheduleItem.time.c_str());
#ifdef EMESESP_DEBUG
            // EMSESP::logger().debug("condition match: %s", match.c_str());
#endif
            if (match.length() == 1 && match[0] == '1' && scheduleItem.retry_cnt == 0xFF) {
                scheduleItem.retry_cnt = command(scheduleItem.name, scheduleItem.cmd.c_str(), compute(scheduleItem.value.c_str())) ? 1 : 0xFF;
            } else if (match.length() == 1 && match[0] == '0' && scheduleItem.retry_cnt == 1) {
                scheduleItem.retry_cnt = 0xFF;
            } else if (match.length() != 1) { // the match is not boolean
#if defined(EMSESP_DEBUG)
                EMSESP::logger().debug("condition result: %s", match.c_str());
#endif
            }
        }
    }
}

// process any scheduled jobs
void WebSchedulerService::loop() {
    // initialize static value on startup
    static int8_t   last_tm_min     = -2; // invalid value also used for startup commands
    static uint32_t last_uptime_min = 0;
    static uint32_t last_uptime_sec = 0;

    if (!raw_value.empty()) { // process a value from system/message command
        computed_value = compute(raw_value);
        raw_value.clear();
    }

    // get list of scheduler events and exit if it's empty
    if (scheduleItems_->empty()) {
        return;
    }

    // check if we have onChange events
    while (!cmd_changed_.empty()) {
        ScheduleItem si = *cmd_changed_.front();
        command(si.name, si.cmd.c_str(), compute(si.value.c_str()));
        cmd_changed_.pop_front();
    }

    for (ScheduleItem & scheduleItem : *scheduleItems_) {
        if (scheduleItem.active && scheduleItem.flags == SCHEDULEFLAG_SCHEDULE_IMMEDIATE) {
            command(scheduleItem.name, scheduleItem.cmd.c_str(), compute(scheduleItem.value.c_str()));
            scheduleItem.active = false;
            publish_single(scheduleItem.name, false);
            if (EMSESP::mqtt_.get_publish_onchange(0)) {
                publish();
            }
        }
    }

    // check conditions every 10 seconds, start after one minute
    uint32_t uptime_sec = uuid::get_uptime_sec() / 10;
    if (last_uptime_sec != uptime_sec && uptime_sec > 5) {
        condition();
        last_uptime_sec = uptime_sec;
    }

    // check startup commands
    if (last_tm_min == -2) {
        for (ScheduleItem & scheduleItem : *scheduleItems_) {
            if (scheduleItem.active && scheduleItem.flags == SCHEDULEFLAG_SCHEDULE_TIMER && scheduleItem.elapsed_min == 0) {
                scheduleItem.retry_cnt = command(scheduleItem.name, scheduleItem.cmd.c_str(), compute(scheduleItem.value.c_str())) ? 0xFF : 0;
            }
        }
        last_tm_min = -1; // startup done, now use for RTC
    }

    // check timer every minute, sync to EMS-ESP clock
    uint32_t uptime_min = uuid::get_uptime_sec() / 60;
    if (last_uptime_min != uptime_min) {
        for (ScheduleItem & scheduleItem : *scheduleItems_) {
            // retry startup commands not yet executed
            if (scheduleItem.active && scheduleItem.flags == SCHEDULEFLAG_SCHEDULE_TIMER && scheduleItem.elapsed_min == 0
                && scheduleItem.retry_cnt < MAX_STARTUP_RETRIES) {
                scheduleItem.retry_cnt = command(scheduleItem.name, scheduleItem.cmd.c_str(), scheduleItem.value.c_str()) ? 0xFF : scheduleItem.retry_cnt + 1;
            }
            // scheduled timer commands
            if (scheduleItem.active && scheduleItem.flags == SCHEDULEFLAG_SCHEDULE_TIMER && scheduleItem.elapsed_min > 0
                && (uptime_min % scheduleItem.elapsed_min == 0)) {
                command(scheduleItem.name, scheduleItem.cmd.c_str(), compute(scheduleItem.value.c_str()));
            }
        }
        last_uptime_min = uptime_min;
    }

    // check calender, sync to RTC, only execute if year is valid
    time_t now = time(nullptr);
    tm *   tm  = localtime(&now);
    if (tm->tm_min != last_tm_min && tm->tm_year > 120) {
        // find the real dow and minute from RTC
        uint8_t  real_dow = 1 << tm->tm_wday; // 1 is Sunday
        uint16_t real_min = tm->tm_hour * 60 + tm->tm_min;

        for (const ScheduleItem & scheduleItem : *scheduleItems_) {
            uint8_t dow = scheduleItem.flags & SCHEDULEFLAG_SCHEDULE_TIMER ? 0 : scheduleItem.flags;
            if (scheduleItem.active && (real_dow & dow) && real_min == scheduleItem.elapsed_min) {
                command(scheduleItem.name, scheduleItem.cmd.c_str(), compute(scheduleItem.value.c_str()));
            }
        }
        last_tm_min = tm->tm_min;
    }
}

// process schedules async
void WebSchedulerService::scheduler_task(void * pvParameters) {
    while (1) {
        delay(10); // no need to hurry
        if (EMSESP::system_.systemStatus() == SYSTEM_STATUS::SYSTEM_STATUS_NORMAL) {
            EMSESP::webSchedulerService.loop();
        }
    }
#ifndef EMSESP_STANDALONE
    vTaskDelete(NULL);
#endif
}

// hard coded tests
#if defined(EMSESP_TEST)
void WebSchedulerService::load_test_data() {
    update([&](WebScheduler & webScheduler) {
        webScheduler.scheduleItems.clear(); // delete all existing schedules

        // test 1
        auto si   = ScheduleItem();
        si.active = true;
        si.flags  = 1;
        si.time   = "12:00";
        si.cmd    = "system/fetch";
        si.value  = "10";
        strcpy(si.name, "test_scheduler");
        si.elapsed_min = 0;
        si.retry_cnt   = 0xFF; // no startup retries

        webScheduler.scheduleItems.push_back(si);

        // test 2
        si        = ScheduleItem();
        si.active = false;
        si.flags  = 1;
        si.time   = "13:00";
        si.cmd    = "system/message";
        si.value  = "20";
        strcpy(si.name, ""); // to make sure its excluded from Dashboard
        si.elapsed_min = 0;
        si.retry_cnt   = 0xFF; // no startup retries

        webScheduler.scheduleItems.push_back(si);

        return StateUpdateResult::CHANGED; // persist the changes
    });
}
#endif

} // namespace emsesp
