/**
 * @file WebServer.cpp
 * @brief Implementation of the WebServer class methods.
 */

#include "WebServer.h"
#include <utility>

WebServer::WebServer(int id) {
    this->id = id;
    this->cyclesRemaining = -1; // no request
}

int WebServer::getId() const noexcept {
    return id;
}

std::optional<Request> WebServer::tick() {
    if (!currRequest.has_value()) {
        return std::nullopt;
    }

    cyclesRemaining--;

    if (cyclesRemaining <= 0) {
        Request finished = std::move(*currRequest);
        currRequest.reset();
        return finished;
    }

    return std::nullopt;
}

void WebServer::assignRequest(Request req) {
    cyclesRemaining = req.time;
    currRequest = std::move(req);
}