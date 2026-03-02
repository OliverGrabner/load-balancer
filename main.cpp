#include "LoadBalancer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

int main() {

    // Config vals to init
    int queueMin, queueMax, scalingCooldown, minRequestTime, maxRequestTime;
    double newRequestProb;
    std::string logFile, blockedStart, blockedEnd;
    bool askUserInput;

    // Read config.txt
    std::ifstream configFile("config.txt");
    std::string line;

    while (std::getline(configFile, line)) {
        int eqPos = line.find('=');
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
    }
    configFile.close();

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