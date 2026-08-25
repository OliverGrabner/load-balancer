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
        std::vector<std::pair<uint32_t, uint32_t>> firewallRange; ///< Blocked IP ranges (start, end), precomputed at insert time

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

        std::vector<int> waitCycles;    ///< Wait time (assignment - arrival) for each completed request
        std::vector<int> sojournCycles; ///< Total time in system (completion - arrival) for each completed request

        bool enableScaling;        ///< If false, checkScaling() is a no-op (fixed fleet size)
        int initialQueueMultiplier; ///< initializeQueue() seeds servers * this many requests
        int warmupCycles;          ///< Completions before this cycle are excluded from latency stats
        bool quiet;                ///< If true, log() is a no-op (suppresses file and console output)

        std::ofstream logFile; ///< Output stream for the log file

        /**
         * @brief Checks if an IP address falls within any blocked firewall range.
         * @param ip The IP address to check
         * @return true if the IP is blocked, false otherwise
         */
        bool isBlockedIP(uint32_t ip) const;

        /**
         * @brief Returns the p-th percentile (nearest-rank) of a pre-sorted vector of values.
         * @param sortedValues Values sorted in ascending order
         * @param p Percentile in [0, 100]
         * @return The value at that percentile, or 0 if the vector is empty
         */
        static int percentile(const std::vector<int>& sortedValues, double p);

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
         * @param enableScaling If false, the fleet size stays fixed at numServers
         * @param initialQueueMultiplier initializeQueue() seeds numServers * this many requests
         * @param warmupCycles Completions before this cycle are excluded from latency stats
         * @param quiet If true, suppresses file and console log output
         */
        LoadBalancer(int numServers, int maxTime, int queueMin, int queueMax,
                    int scalingCooldown, double newRequestProb,
                    int minRequestTime, int maxRequestTime,
                    const std::string& logFileName,
                    bool enableScaling = true, int initialQueueMultiplier = 100,
                    int warmupCycles = 0, bool quiet = false);

        /**
         * @brief Destructor. Frees all server instances and closes the log file.
         */
        ~LoadBalancer();

        // Not copyable: a copy would double-delete the server pool.
        LoadBalancer(const LoadBalancer&) = delete;
        LoadBalancer& operator=(const LoadBalancer&) = delete;

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
        int getQueueSize() const noexcept;

        /**
         * @brief Returns the current number of active servers.
         * @return The server count
         */
        int getServerCount() const noexcept;

        /**
         * @brief Returns the p-th percentile of queue wait time (assignment - arrival), in cycles.
         * @param p Percentile in [0, 100]
         */
        int getWaitPercentile(double p) const;

        /**
         * @brief Returns the p-th percentile of sojourn time (completion - arrival), in cycles.
         * @param p Percentile in [0, 100]
         */
        int getSojournPercentile(double p) const;
};

#endif
