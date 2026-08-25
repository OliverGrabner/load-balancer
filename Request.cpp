/**
 * @file Request.cpp
 * @brief Implementation of the Request struct methods.
 */

#include "Request.h"
#include <random>

static std::mt19937 rng;

Request::Request(std::string ipIn, std::string ipOut, int time, char jobType) {
    this->ipIn = ipIn;
    this->ipOut = ipOut;
    this->time = time;
    this->jobType = jobType;
}

void Request::seed(unsigned int seed) {
    rng.seed(seed);
}

int Request::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

double Request::randomDouble() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

std::string Request::generateRandomIP() {
    return std::to_string(randomInt(0, 255)) + "." +
           std::to_string(randomInt(0, 255)) + "." +
           std::to_string(randomInt(0, 255)) + "." +
           std::to_string(randomInt(0, 255));
}

Request Request::generateRandom(int minTime, int maxTime) {
    std::string ipIn = generateRandomIP();
    std::string ipOut = generateRandomIP();
    int time = randomInt(minTime, maxTime);
    char jobType = (randomInt(0, 1) == 0) ? 'P' : 'S';
    return Request(ipIn, ipOut, time, jobType);
}