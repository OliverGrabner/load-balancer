/**
 * @file LoadBalancer.cpp
 * @brief Implementation of the LoadBalancer class methods.
 */

#include "LoadBalancer.h"
#include <iostream>
#include <sstream>

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
                           const std::string& logFileName) {

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

    logFile.open(logFileName);
    
    // create servers 
    for(int i = 0; i < numServers; i++) {
        WebServer* server = new WebServer(nextServerId++);
        servers.push_back(server);
        availableQueue.push(server);
    }

    log("LoadBalancer created with " + std::to_string(numServers) + " servers", CYAN);

}

LoadBalancer::~LoadBalancer() {
    for (int i = 0; i < servers.size(); i++) delete servers[i];
    servers.clear();

    if (logFile.is_open()) logFile.close();
}

// getter functions 
int LoadBalancer::getQueueSize() const { return requestQueue.size(); } 
int LoadBalancer::getServerCount() const { return servers.size(); }

// FIREWALL
void LoadBalancer::addBlockedIP(const std::string& startIP, const std::string& stopIP) {
    firewallRange.push_back(std::make_pair(startIP, stopIP));
    log("Blocked IP range: " + startIP + " - " + stopIP, RED);
}

unsigned long LoadBalancer::ipToLong(const std::string& ip) {
    unsigned long result = 0;
    unsigned long octet = 0;
    int shift = 24;

    for (int i = 0; i < ip.length(); i++) {
        if (ip[i] == '.') {
            result |= (octet << shift);
            shift -= 8;
            octet = 0;
        } else {
            octet = octet * 10 + (ip[i] - '0');
        }
    }
    result |= (octet << shift);

    return result;
}

bool LoadBalancer::isBlockedIP(const std::string& ip) const {
    unsigned long ipNum = ipToLong(ip);

    for (int i = 0; i < firewallRange.size(); i++) {
        unsigned long start = ipToLong(firewallRange[i].first);
        unsigned long end = ipToLong(firewallRange[i].second);
        if (ipNum >= start && ipNum <= end) {
            return true;
        }
    }
    // found no blocks, so it works 
    return false;
}

// Queue 

void LoadBalancer::initializeQueue() {
    int count = servers.size() * 100;

    for (int i = 0; i < count; i++) {
        Request req = Request::generateRandom(minRequestTime, maxRequestTime);

        if (!isBlockedIP(req.ipIn)) {
            requestQueue.push(req);
            log("Request #" + std::to_string(i) + ": Added (" + req.ipIn + ")", CYAN);
        } else { // reject and dont put in 
            totalRejectedRequests++;
            log("Request #" + std::to_string(i) + ": REJECTED because of bad IP (" + req.ipIn + ")", RED);
        }
    }

    startingQueueSize = requestQueue.size();
    log("Queue initialized with " + std::to_string(startingQueueSize) + " requests", CYAN);
}


// methods in cycle 

void LoadBalancer::generateNewRequest() {
    double roll = (double)rand() / RAND_MAX;

    if (roll < generateRequestProbability) {
        Request req = Request::generateRandom(minRequestTime, maxRequestTime);

        if (isBlockedIP(req.ipIn)) {
            totalRejectedRequests++;
            log("[Cycle " + std::to_string(currTime) + "] BLOCKED request from " + req.ipIn, RED);
        } else {
            requestQueue.push(req);
        }
    }
}

// go through each webserver and tick, if a webserver finishes a request, it becomes available again
void LoadBalancer::tickAllServers() {
    for (int i = 0; i < servers.size(); i++) {
        if (servers[i]->tick()) { // become available 
            totalProcessedRequests++;
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

        server->assignRequest(new Request(req));
        log("[Cycle " + std::to_string(currTime) + "] Assigned request to Server " +
            std::to_string(server->getId()) + " (time: " + std::to_string(req.time) +
            ", type: " + req.jobType + ")", BLUE);
    }
}

// if queu size goes above what it should be , add a server, wait n clock cycles
void LoadBalancer::checkScaling() {
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
        for (int i = 0; i < servers.size(); i++) {
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
    log("=================================================");
}