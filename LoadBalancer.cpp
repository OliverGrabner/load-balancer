/**
 * @file LoadBalancer.cpp
 * @brief Implementation of the LoadBalancer class methods.
 */

#include "LoadBalancer.h"
#include <iostream>
#include <sstream>
#include <algorithm>

// COLORS :P
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string RESET = "\033[0m";

LoadBalancer::LoadBalancer(int numServers, int maxTime, int queueMin, int queueMax,
                           int scalingCooldown, double newRequestProb,
                           int minRequestTime, int maxRequestTime,
                           const std::string& logFileName,
                           bool enableScaling, int initialQueueMultiplier,
                           int warmupCycles, bool quiet) {

    this->currTime = 0;
    this->maxTime = maxTime;
    this->cooldownRemaining = 0;
    this->nextServerId = 0;
    this->queueMin = queueMin;
    this->queueMax = queueMax;
    this->scalingCooldown = scalingCooldown;
    this->generateRequestProbability = newRequestProb;
    this->minRequestTime = minRequestTime;
    this->maxRequestTime = maxRequestTime;
    this->totalProcessedRequests = 0;
    this->totalRejectedRequests = 0;
    this->serversAdded = 0;
    this->serversRemoved = 0;
    this->startingQueueSize = 0;
    this->enableScaling = enableScaling;
    this->initialQueueMultiplier = initialQueueMultiplier;
    this->warmupCycles = warmupCycles;
    this->quiet = quiet;

    logFile.open(logFileName);
    
    // create servers 
    for(int i = 0; i < numServers; i++) {
        WebServer* server = new WebServer(nextServerId++);
        servers.push_back(server);
        availableQueue.push(server);
    }

    log("LoadBalancer created with " + std::to_string(numServers) + " servers", CYAN);
    log("LoadBalancer running for " + std::to_string(maxTime) + " Cycles", CYAN);

}

LoadBalancer::~LoadBalancer() {
    for (size_t i = 0; i < servers.size(); i++) delete servers[i];
    servers.clear();

    if (logFile.is_open()) logFile.close();
}

// getter functions
int LoadBalancer::getQueueSize() const noexcept { return requestQueue.size(); }
int LoadBalancer::getServerCount() const noexcept { return servers.size(); }

int LoadBalancer::getWaitPercentile(double p) const {
    std::vector<int> sorted = waitCycles;
    std::sort(sorted.begin(), sorted.end());
    return percentile(sorted, p);
}

int LoadBalancer::getSojournPercentile(double p) const {
    std::vector<int> sorted = sojournCycles;
    std::sort(sorted.begin(), sorted.end());
    return percentile(sorted, p);
}

// FIREWALL
void LoadBalancer::addBlockedIP(const std::string& startIP, const std::string& stopIP) {
    firewallRange.push_back(std::make_pair(ipToLong(startIP), ipToLong(stopIP)));
    log("Blocked IP range: " + startIP + " - " + stopIP, RED);
}

int LoadBalancer::percentile(const std::vector<int>& sortedValues, double p) {
    if (sortedValues.empty()) return 0;

    size_t idx = (size_t)(p / 100.0 * sortedValues.size());
    if (idx >= sortedValues.size()) idx = sortedValues.size() - 1;

    return sortedValues[idx];
}

bool LoadBalancer::isBlockedIP(uint32_t ip) const {
    for (size_t i = 0; i < firewallRange.size(); i++) {
        if (ip >= firewallRange[i].first && ip <= firewallRange[i].second) {
            return true;
        }
    }
    // found no blocks, so it works
    return false;
}

// Queue 

void LoadBalancer::initializeQueue() {
    int count = servers.size() * initialQueueMultiplier;

    for (int i = 0; i < count; i++) {
        Request req = Request::generateRandom(minRequestTime, maxRequestTime);
        req.arrivalCycle = currTime;

        if (!isBlockedIP(req.ipIn)) {
            requestQueue.push(req);
            log("Request #" + std::to_string(i) + ": Added (" + ipToString(req.ipIn) + ")", CYAN);
        } else { // reject and dont put in
            totalRejectedRequests++;
            log("Request #" + std::to_string(i) + ": REJECTED because of bad IP (" + ipToString(req.ipIn) + ")", RED);
        }
    }

    startingQueueSize = requestQueue.size();
    log("Queue initialized with " + std::to_string(startingQueueSize) + " requests", CYAN);
}


// methods in cycle 

void LoadBalancer::generateNewRequest() {
    double roll = Request::randomDouble();

    if (roll < generateRequestProbability) {
        Request req = Request::generateRandom(minRequestTime, maxRequestTime);
        req.arrivalCycle = currTime;

        if (isBlockedIP(req.ipIn)) {
            totalRejectedRequests++;
            log("[Cycle " + std::to_string(currTime) + "] BLOCKED request from " + ipToString(req.ipIn), RED);
        } else {
            requestQueue.push(req);
        }
    }
}

// go through each webserver and tick, if a webserver finishes a request, it becomes available again
void LoadBalancer::tickAllServers() {
    for (size_t i = 0; i < servers.size(); i++) {
        if (auto finished = servers[i]->tick()) { // returns the completed request, if any
            totalProcessedRequests++;
            if (currTime >= warmupCycles) {
                waitCycles.push_back(finished->startCycle - finished->arrivalCycle);
                sojournCycles.push_back(currTime - finished->arrivalCycle);
            }

            availableQueue.push(servers[i]);
            log("[Cycle " + std::to_string(currTime) + "] Server " +
                std::to_string(servers[i]->getId()) + " finished a request", GREEN);
        }
    }
}


// FIFO queue, assigns a server to the request until servers all full, or requests empty
void LoadBalancer::assignRequest() {
    while (!availableQueue.empty() && !requestQueue.empty()) {
        WebServer* server = availableQueue.front();
        availableQueue.pop();

        Request req = requestQueue.front();
        requestQueue.pop();
        req.startCycle = currTime;

        server->assignRequest(req);
        log("[Cycle " + std::to_string(currTime) + "] Assigned request to Server " +
            std::to_string(server->getId()) + " (time: " + std::to_string(req.time) +
            ", type: " + jobTypeChar(req.jobType) + ")", BLUE);
    }
}

// if queu size goes above what it should be , add a server, wait n clock cycles
void LoadBalancer::checkScaling() {
    if (!enableScaling) return;

    // tick down
    if (cooldownRemaining > 0) {
        cooldownRemaining--;
        return;
    }

    int queueSize = requestQueue.size();
    int numServers = servers.size();

    // high traffic - add a server
    if (queueSize > queueMax * numServers) {
        WebServer* server = new WebServer(nextServerId++);
        servers.push_back(server);
        availableQueue.push(server);
        serversAdded++;
        cooldownRemaining = scalingCooldown;

        log("[Cycle " + std::to_string(currTime) + "] SCALED UP: added Server " +
            std::to_string(server->getId()) + " (queue: " + std::to_string(queueSize) +
            ", servers: " + std::to_string(servers.size()) + ")", YELLOW);
    }
    // low traffic - remove UNUSED server
    else if (queueSize < queueMin * numServers && numServers > 1) {
        // Only remove a server that is idle (in the available queue)
        if (availableQueue.empty()) {
            return;  // all servers busy, can't remove any
        }

        // Pop an idle server from the available queue
        WebServer* server = availableQueue.front();
        availableQueue.pop();

        // Remove it from the servers vector
        for (size_t i = 0; i < servers.size(); i++) {
            if (servers[i] == server) {
                servers.erase(servers.begin() + i);
                break;
            }
        }

        int removedId = server->getId();
        delete server;
        serversRemoved++;
        cooldownRemaining = scalingCooldown;

        log("[Cycle " + std::to_string(currTime) + "] SCALED DOWN: removed Server " +
            std::to_string(removedId) + " (queue: " + std::to_string(queueSize) +
            ", servers: " + std::to_string(servers.size()) + ")", MAGENTA);
    }
}

// log to logfile and print in terminal message
void LoadBalancer::log(const std::string& message, const std::string& color) {
    if (quiet) return;

    if (logFile.is_open()) {
        logFile << message << std::endl;
    }

    if (color.empty()) {
        std::cout << message << std::endl;
    } else {
        std::cout << color << message << RESET << std::endl;
    }
}

// main simulation run 
void LoadBalancer::run() {
    log("=== Simulation Starting ===", CYAN);
    log("Servers: " + std::to_string(servers.size()), CYAN);
    log("Max time: " + std::to_string(maxTime) + " cycles", CYAN);
    log("Queue size: " + std::to_string(requestQueue.size()), CYAN);
    log("Task time range: " + std::to_string(minRequestTime) + " - " + std::to_string(maxRequestTime), CYAN);
    log("");
    log("");
    log("");

    log("=== Simulation Running ===", CYAN);

    for (currTime = 0; currTime < maxTime; currTime++) {
        generateNewRequest(); // create new request (% chance of occurance) 
        tickAllServers(); // tick servers and change availability 
        assignRequest(); // assign requests in queue to servers
        checkScaling(); // if overclocked, add more servers, do reverse as well
    }

    log("");
    log("");
    log("");
    log("=== Simulation Complete ===", CYAN);

}

void LoadBalancer::printSummary() {
    int idleCount = availableQueue.size();
    int busyCount = servers.size() - idleCount;

    log("");
    log("=== Summary & End of Simulation STATS ===", CYAN);
    log("Starting queue size: " + std::to_string(startingQueueSize));
    log("Ending queue size: " + std::to_string(requestQueue.size()));
    log("Total processed: " + std::to_string(totalProcessedRequests));
    log("Total rejected: " + std::to_string(totalRejectedRequests));
    log("Task time range: " + std::to_string(minRequestTime) + " - " + std::to_string(maxRequestTime));
    log("Servers added: " + std::to_string(serversAdded));
    log("Servers removed: " + std::to_string(serversRemoved));
    log("Final server count: " + std::to_string(servers.size()));
    log("Busy servers: " + std::to_string(busyCount));
    log("Idle servers: " + std::to_string(idleCount));

    log("");
    log("Wait time (cycles, queue to assignment) over " + std::to_string(waitCycles.size()) + " completed requests:");
    log("  p50: " + std::to_string(getWaitPercentile(50)) +
        "  p95: " + std::to_string(getWaitPercentile(95)) +
        "  p99: " + std::to_string(getWaitPercentile(99)) +
        "  max: " + std::to_string(getWaitPercentile(100)));
    log("Sojourn time (cycles, arrival to completion) over " + std::to_string(sojournCycles.size()) + " completed requests:");
    log("  p50: " + std::to_string(getSojournPercentile(50)) +
        "  p95: " + std::to_string(getSojournPercentile(95)) +
        "  p99: " + std::to_string(getSojournPercentile(99)) +
        "  max: " + std::to_string(getSojournPercentile(100)));
    log("=================================================");
}