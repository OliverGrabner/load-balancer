#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include "Request.h"
#include "WebServer.h"

#include <queue>
#include <string>
#include <cstdlib>
#include <fstream>

class LoadBalancer {
    private:
        std::queue<Request> requestQueue;
        std::queue<WebServer*> availableQueue;  // active looking for jobs
        std::vector<WebServer*> servers; // list of all the webservers (busy and idle)
        std::vector<std::pair<std::string, std::string>> firewallRange;

        int currTime;
        int maxTime; // total run cycles
        int cooldownRemaining;
        int queueMin;
        int queueMax;
        double generateRequestProbablilty;
        int minRequestTime;
        int maxRequestTime;

        // end stats
        int totalProcessedRequests;
        int totalRejectedRequests;
        int serversAdded;
        int serversRemoved;
        int avgWaitTime; // average request wait time to be started
        std::ofstream logFile;

        // priv methods 
        const bool isBlockedIP(const std::string& ip);
        static unsigned long ipToLong(const std::string& ip);
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
            void addBlockedIP(const std::string startIP, const std::string stopIP);
            void initializeQueue();
            void run();
            void printSummary();
            int getQueueSize();
            int getServerCount();

};
#endif 