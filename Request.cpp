/**
 * @file Request.cpp
 * @brief Implementation of the Request struct methods.
 */

#include "Request.h"
#include <random>

static std::mt19937 rng;

char jobTypeChar(JobType jobType) {
    return jobType == JobType::Processing ? 'P' : 'S';
}

uint32_t ipToLong(const std::string& ip) {
    uint32_t result = 0;
    uint32_t octet = 0;
    int shift = 24;

    for (size_t i = 0; i < ip.length(); i++) {
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

std::string ipToString(uint32_t ip) {
    return std::to_string((ip >> 24) & 0xFF) + "." +
           std::to_string((ip >> 16) & 0xFF) + "." +
           std::to_string((ip >> 8) & 0xFF) + "." +
           std::to_string(ip & 0xFF);
}

Request::Request(uint32_t ipIn, uint32_t ipOut, int time, JobType jobType) {
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

uint32_t Request::generateRandomIP() {
    uint32_t octet1 = (uint32_t)randomInt(0, 255);
    uint32_t octet2 = (uint32_t)randomInt(0, 255);
    uint32_t octet3 = (uint32_t)randomInt(0, 255);
    uint32_t octet4 = (uint32_t)randomInt(0, 255);
    return (octet1 << 24) | (octet2 << 16) | (octet3 << 8) | octet4;
}

Request Request::generateRandom(int minTime, int maxTime) {
    uint32_t ipIn = generateRandomIP();
    uint32_t ipOut = generateRandomIP();
    int time = randomInt(minTime, maxTime);
    JobType jobType = (randomInt(0, 1) == 0) ? JobType::Processing : JobType::Streaming;
    return Request(ipIn, ipOut, time, jobType);
}