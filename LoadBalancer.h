/**
 * @file LoadBalancer.h
 * @brief Defines the LoadBalancer class that manages servers and the request queue.
 */

#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include "Request.h"
#include "WebServer.h"

#include <queue>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <utility>

/**
 * @brief Manages a pool of web servers and a queue of incoming requests.
 *
 * The LoadBalancer distributes requests to available servers using a FIFO queue,
 * dynamically scales the server pool based on queue thresholds, and provides
 * IP-based firewall filtering to block requests from specified address ranges.
 */
class LoadBalancer {
    private:
        std::queue<Request> requestQueue;       ///< FIFO queue of pending requests
        std::queue<WebServer*> availableQueue;  ///< Queue of idle servers ready for assignment
        std::vector<WebServer*> servers;        ///< All active server instances
        std::vector<std::pair<std::string, std::string>> firewallRange; ///< Blocked IP ranges (start, end)

        int currTime;           ///< Current simulation clock cycle
        int maxTime;            ///< Total number of clock cycles to simulate
        int cooldownRemaining;  ///< Clock cycles remaining before next scaling action
        int nextServerId;       ///< ID to assign to the next created server

        // Config values
        int queueMin;                       ///< Lower queue threshold per server for scaling down
        int queueMax;                       ///< Upper queue threshold per server for scaling up
        int scalingCooldown;                ///< Cooldown period (cycles) between scaling actions
        double generateRequestProbability;  ///< Probability of generating a new request each cycle
        int minRequestTime;                 ///< Minimum processing time for generated requests
        int maxRequestTime;                 ///< Maximum processing time for generated requests

        // End-of-simulation statistics
        int totalProcessedRequests; ///< Total requests successfully processed
        int totalRejectedRequests;  ///< Total requests rejected by the firewall
        int serversAdded;           ///< Number of servers added during simulation
        int serversRemoved;         ///< Number of servers removed during simulation
        int startingQueueSize;      ///< Queue size after initial population

        std::ofstream logFile; ///< Output stream for the log file

        /**
         * @brief Checks if an IP address falls within any blocked firewall range.
         * @param ip The IP address to check
         * @return true if the IP is blocked, false otherwise
         */
        bool isBlockedIP(const std::string& ip) const;

        /**
         * @brief Converts a dotted-decimal IP address string to an unsigned long.
         * @param ip The IP address string (e.g. "192.168.1.1")
         * @return The numeric representation of the IP address
         */
        static unsigned long ipToLong(const std::string& ip);

        /**
         * @brief Checks queue thresholds and adds or removes servers as needed.
         */
        void checkScaling();

        /**
         * @brief Randomly generates a new request and adds it to the queue (based on probability).
         */
        void generateNewRequest();

        /**
         * @brief Assigns pending requests from the queue to available servers.
         */
        void assignRequest();

        /**
         * @brief Advances all servers by one clock cycle and marks finished servers as available.
         */
        void tickAllServers();

        /**
         * @brief Logs a message to the log file and prints it to the console.
         * @param message The message to log
         * @param color ANSI color code for console output (empty string for no color)
         */
        void log(const std::string& message, const std::string& color = "");

    public:
        /**
         * @brief Constructs a LoadBalancer with the given configuration.
         * @param numServers Initial number of web servers to create
         * @param maxTime Total simulation time in clock cycles
         * @param queueMin Lower queue threshold per server for scaling down
         * @param queueMax Upper queue threshold per server for scaling up
         * @param scalingCooldown Cooldown period between scaling actions
         * @param newRequestProb Probability of generating a new request each cycle
         * @param minRequestTime Minimum processing time for requests
         * @param maxRequestTime Maximum processing time for requests
         * @param logFileName Path to the output log file
         */
        LoadBalancer(int numServers, int maxTime, int queueMin, int queueMax,
                    int scalingCooldown, double newRequestProb,
                    int minRequestTime, int maxRequestTime,
                    const std::string& logFileName);

        /**
         * @brief Destructor. Frees all server instances and closes the log file.
         */
        ~LoadBalancer();

        /**
         * @brief Adds a blocked IP address range to the firewall.
         * @param startIP Start of the blocked IP range
         * @param stopIP End of the blocked IP range
         */
        void addBlockedIP(const std::string& startIP, const std::string& stopIP);

        /**
         * @brief Populates the request queue with an initial batch of requests (servers * 100).
         */
        void initializeQueue();

        /**
         * @brief Runs the load balancer simulation for the configured number of clock cycles.
         */
        void run();

        /**
         * @brief Prints a summary of simulation statistics to the log and console.
         */
        void printSummary();

        /**
         * @brief Returns the current number of pending requests in the queue.
         * @return The request queue size
         */
        int getQueueSize() const;

        /**
         * @brief Returns the current number of active servers.
         * @return The server count
         */
        int getServerCount() const;
};

#endif
