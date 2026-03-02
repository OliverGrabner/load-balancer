/**
 * @file WebServer.h
 * @brief Defines the WebServer class that processes requests assigned by the LoadBalancer.
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "Request.h"

/**
 * @brief Simulates a web server that receives and processes requests from the load balancer.
 *
 * Each WebServer processes one request at a time. It counts down clock cycles
 * until the request is complete, then becomes available for a new request.
 */
class WebServer {
    private:
        int id;                 ///< Unique identifier for this server
        Request* currRequest;   ///< Pointer to the currently assigned request, or nullptr if idle
        int cyclesRemaining;    ///< Clock cycles remaining to finish the current request

    public:
        /**
         * @brief Constructs a WebServer with the given ID.
         * @param id Unique server identifier
         */
        WebServer(int id);

        /**
         * @brief Destructor. Cleans up any assigned request to prevent memory leaks.
         */
        ~WebServer();

        /**
         * @brief Returns the server's unique ID.
         * @return The server ID
         */
        int getId();

        /**
         * @brief Advances the server by one clock cycle.
         * @return true if the server just finished processing its request, false otherwise
         */
        const bool tick();

        /**
         * @brief Assigns a new request to this server for processing.
         * @param req Pointer to the Request to process (takes ownership)
         */
        void assignRequest(Request* req);
};

#endif
