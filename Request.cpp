#include "Request.h"
#include <cstdlib>

Request::Request(std::string ipIn, std::string ipOut, int time, char jobType) {
    this->ipIn = ipIn;
    this->ipOut = ipOut;
    this->time = time;
    this->jobType = jobType;
}

std::string Request::generateRandomIP() {
    return std::to_string(rand() % 256) + "." +
           std::to_string(rand() % 256) + "." +
           std::to_string(rand() % 256) + "." +
           std::to_string(rand() % 256);
}

Request Request::generateRandom(int minTime, int maxTime) {
    std::string ipIn = generateRandomIP();
    std::string ipOut = generateRandomIP();
    int time = minTime + (rand() % (maxTime - minTime + 1));
    char jobType = (rand() % 2 == 0) ? 'P' : 'S';
    return Request(ipIn, ipOut, time, jobType);
}