#include "WebServer.h"

WebServer::WebServer(int id) {
    this->id = id;
    this->currRequest = nullptr;
    this->cyclesRemaining = -1; // no request
}

WebServer::~WebServer() {
    // should NEVER be the case since we only take away from the active pool. but we handle just in case due to mem leaks
    if (currRequest != nullptr){
        delete currRequest;
        currRequest = nullptr;
    }
}

int WebServer::getId() {
    return id;
}

const bool WebServer::tick() {
    if (currRequest == nullptr) {
        return false;
    }

    cyclesRemaining--;

    if (cyclesRemaining <= 0) {
        delete currRequest;
        currRequest = nullptr;
        return true;
    }

    return false;
}

void WebServer::assignRequest(Request* req) {
    currRequest = req;
    cyclesRemaining = req->time;
}