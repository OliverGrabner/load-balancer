/*
WEBSERVER MUST
Take request from LB
Process Request
Become Available again
*/
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "Request.h"

class WebServer {
    private:
        int id;
        Request* currRequest;
        int cyclesRemaining;

    public:
        WebServer(int id);
        ~WebServer();
        int getId();
        const bool tick(); // return true if Request finisehd 
        void assignRequest(Request* req);
};

#endif 