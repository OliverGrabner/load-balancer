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

class LoadBalancer {
    private:
        std::queue<Request> requestQueue;
        std::queue<WebServer*> availableQueue;
        std::vector<WebServer*> servers;
        std::vector<std::pair<std::string, std::string>> firewallRange;

        int currTime;
        int maxTime; // total run cycles
        int cooldownRemaining;
        int nextServerId;

        // config.json vals
        int queueMin;
        int queueMax;
        int scalingCooldown;
        double generateRequestProbability;
        int minRequestTime;
        int maxRequestTime;

        // end stats
        int totalProcessedRequests;
        int totalRejectedRequests;
        int serversAdded;
        int serversRemoved;
        int startingQueueSize;

        // Logging
        std::ofstream logFile;

        // Private methods
        bool isBlockedIP(const std::string& ip) const;
        static unsigned long ipToLong(const std::string& ip);
        void checkScaling();
        void generateNewRequest();
        void assignRequest();
        void tickAllServers();
        void log(const std::string& message, const std::string& color = "");

    public:
        LoadBalancer(int numServers, int maxTime, int queueMin, int queueMax,
                    int scalingCooldown, double newRequestProb,
                    int minRequestTime, int maxRequestTime,
                    const std::string& logFileName);

        ~LoadBalancer();

        void addBlockedIP(const std::string& startIP, const std::string& stopIP);
        void initializeQueue();
        void run();
        void printSummary();
        int getQueueSize() const;
        int getServerCount() const;
};

#endif