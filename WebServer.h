/**
 * @file WebServer.h
 * @brief Defines the WebServer class that processes requests assigned by the LoadBalancer.
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "Request.h"
#include <optional>

/**
 * @brief Simulates a web server that receives and processes requests from the load balancer.
 *
 * Each WebServer processes one request at a time. It counts down clock cycles
 * until the request is complete, then becomes available for a new request.
 */
class WebServer {
    private:
        int id;                              ///< Unique identifier for this server
        std::optional<Request> currRequest;  ///< The request being processed, or empty if idle
        int cyclesRemaining;                 ///< Clock cycles remaining to finish the current request

    public:
        /**
         * @brief Constructs a WebServer with the given ID.
         * @param id Unique server identifier
         */
        explicit WebServer(int id);

        ~WebServer() = default;

        // Not copyable: each WebServer is a uniquely-identified pool member.
        WebServer(const WebServer&) = delete;
        WebServer& operator=(const WebServer&) = delete;

        /**
         * @brief Returns the server's unique ID.
         * @return The server ID
         */
        int getId() const noexcept;

        /**
         * @brief Advances the server by one clock cycle.
         * @return The completed Request if one just finished this cycle, std::nullopt otherwise.
         */
        std::optional<Request> tick();

        /**
         * @brief Assigns a new request to this server for processing.
         * @param req The Request to process
         */
        void assignRequest(Request req);
};

#endif
