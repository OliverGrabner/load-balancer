/**
 * @file Request.h
 * @brief Defines the Request struct representing a network request in the load balancer simulation.
 */

#ifndef REQUEST_H
#define REQUEST_H

#include <string>

/**
 * @brief Represents a single network request with source/destination IPs, processing time, and job type.
 */
struct Request {
    std::string ipIn;   ///< Source IP address of the request
    std::string ipOut;  ///< Destination IP address for the response
    int time;           ///< Processing time in clock cycles
    char jobType;       ///< Job type: 'P' for processing, 'S' for streaming

    /**
     * @brief Constructs a Request with the given parameters.
     * @param ipIn Source IP address
     * @param ipOut Destination IP address
     * @param time Processing time in clock cycles
     * @param jobType Job type character ('P' or 'S')
     */
    Request(std::string ipIn, std::string ipOut, int time, char jobType);

    /**
     * @brief Generates a random Request with random IPs and a random processing time.
     * @param minTime Minimum processing time
     * @param maxTime Maximum processing time
     * @return A randomly generated Request
     */
    static Request generateRandom(int minTime, int maxTime);

    /**
     * @brief Generates a random IP address string in dotted-decimal format.
     * @return A random IP address (e.g. "192.168.1.42")
     */
    static std::string generateRandomIP();
};

#endif
