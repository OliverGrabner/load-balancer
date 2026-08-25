/**
 * @file main.cpp
 * @brief Entry point for the load balancer simulation.
 *
 * Reads configuration from config.txt, optionally prompts the user for
 * server count and run time, then creates and runs the LoadBalancer simulation.
 */

#include "LoadBalancer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

/**
 * @brief Runs a fixed-fleet utilization sweep: for each target rho (server
 * utilization), reports p50/p95/p99/max sojourn latency. Scaling is disabled
 * and the fleet size is fixed so rho stays at its target for the whole run;
 * the first warmupCycles cycles are excluded from the stats so the empty
 * starting queue doesn't bias the result.
 */
void runUtilizationSweep(int minRequestTime, int maxRequestTime) {
    const int numServers = 45;
    const int totalCycles = 50000;
    const int warmupCycles = 10000;
    const double meanServiceTime = (minRequestTime + maxRequestTime) / 2.0;
    const double rhoTargets[] = {0.50, 0.70, 0.80, 0.90, 0.95, 0.97, 0.99, 0.995};

    std::cout << "rho\tarrivalProb\tp50\tp95\tp99\tmax\n";

    for (double rho : rhoTargets) {
        double arrivalProb = rho * numServers / meanServiceTime;

        Request::seed(42);
        LoadBalancer lb(numServers, totalCycles, /*queueMin*/0, /*queueMax*/0,
                        /*scalingCooldown*/5, arrivalProb,
                        minRequestTime, maxRequestTime, "sweep_log.txt",
                        /*enableScaling*/false, /*initialQueueMultiplier*/0,
                        warmupCycles, /*quiet*/true);
        lb.initializeQueue();
        lb.run();

        std::cout << rho << "\t" << arrivalProb << "\t"
                   << lb.getSojournPercentile(50) << "\t"
                   << lb.getSojournPercentile(95) << "\t"
                   << lb.getSojournPercentile(99) << "\t"
                   << lb.getSojournPercentile(100) << "\n";
    }
}

/**
 * @brief Main function that configures and runs the load balancer simulation.
 * @return 0 on successful completion
 */
int main(int argc, char* argv[]) {

    // Config vals to init (defaults used if a key is missing from config.txt)
    int queueMin = 50, queueMax = 80, scalingCooldown = 5;
    int minRequestTime = 4, maxRequestTime = 100;
    double newRequestProb = 0.70;
    std::string logFile = "log.txt", blockedStart, blockedEnd;
    bool askUserInput = true;
    unsigned int seed = 42;

    // Read config.txt
    std::ifstream configFile("config.txt");
    std::string line;

    while (std::getline(configFile, line)) {
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        if (key == "queueMin") queueMin = std::stoi(value);
        else if (key == "queueMax") queueMax = std::stoi(value);
        else if (key == "scalingCooldown") scalingCooldown = std::stoi(value);
        else if (key == "newRequestProbability") newRequestProb = std::stod(value);
        else if (key == "logFile") logFile = value;
        else if (key == "minRequestTime") minRequestTime = std::stoi(value);
        else if (key == "maxRequestTime") maxRequestTime = std::stoi(value);
        else if (key == "askUserInput") askUserInput = (value == "true");
        else if (key == "blockedStart") blockedStart = value;
        else if (key == "blockedEnd") blockedEnd = value;
        else if (key == "seed") seed = (unsigned int)std::stoul(value);
    }
    configFile.close();

    Request::seed(seed);

    if (argc > 1 && std::string(argv[1]) == "--sweep") {
        runUtilizationSweep(minRequestTime, maxRequestTime);
        return 0;
    }

    // Get user input
    int numServers = 10;
    int runTime = 10000;

    if (askUserInput) {
        std::cout << "Enter number of servers: ";
        std::cin >> numServers;
        std::cout << "Enter simulation time (clock cycles): ";
        std::cin >> runTime;
    }

    // Create, configure, and run
    LoadBalancer lb(numServers, runTime, queueMin, queueMax,
                    scalingCooldown, newRequestProb,
                    minRequestTime, maxRequestTime, logFile);

    lb.addBlockedIP(blockedStart, blockedEnd);
    lb.initializeQueue();
    lb.run();
    lb.printSummary();

    return 0;
}