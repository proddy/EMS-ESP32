#ifndef HttpEndpoint_h
#define HttpEndpoint_h

#include <functional>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "SecurityManager.h"
#include "StatefulService.h"

#define HTTP_ENDPOINT_ORIGIN_ID "http"

template <class T>
class HttpEndpoint {
  protected:
    JsonStateReader<T>   _stateReader;
    JsonStateUpdater<T>  _stateUpdater;
    StatefulService<T> * _statefulService;

  public:
    HttpEndpoint(JsonStateReader<T>      stateReader,
                 JsonStateUpdater<T>     stateUpdater,
                 StatefulService<T> *    statefulService,
                 AsyncWebServer *        server,
                 const char *            servicePath,
                 SecurityManager *       securityManager,
                 AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_ADMIN)
        : _stateReader(stateReader)
        , _stateUpdater(stateUpdater)
        , _statefulService(statefulService) {
        securityManager->addEndpoint(server, servicePath, authenticationPredicate, 
            [this](AsyncWebServerRequest * request, JsonVariant json) { handleRequest(request, json); }, HTTP_ANY);
    }

  protected:
    void handleRequest(AsyncWebServerRequest * request, JsonVariant json) {
        // Handle POST request
        if (request->method() == HTTP_POST) {
            if (!json.is<JsonObject>()) {
                request->send(400);
                return;
            }

            const StateUpdateResult outcome = _statefulService->updateWithoutPropagation(json.as<JsonObjectConst>(), _stateUpdater);

            if (outcome == StateUpdateResult::ERROR) {
                request->send(400);
                return;
            }
            
            if (outcome == StateUpdateResult::CHANGED || outcome == StateUpdateResult::CHANGED_RESTART) {
                request->onDisconnect([this] { _statefulService->callUpdateHandlers(); });
                if (outcome == StateUpdateResult::CHANGED_RESTART) {
                    request->send(205);
                    return;
                }
            }
        }

        // Send current state (for GET or successful POST)
        sendStateResponse(request);
    }

  private:
    void sendStateResponse(AsyncWebServerRequest * request) {
        auto * response = new AsyncJsonResponse(false);
        JsonObject jsonObject = response->getRoot().to<JsonObject>();
        _statefulService->read(jsonObject, _stateReader);
        response->setLength();
        request->send(response);
    }
};

#endif
