/**
 * @file Request.h
 * @brief Defines the Request struct representing a network request in the load balancer simulation.
 */

#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <cstdint>

/**
 * @brief Type of job a request represents.
 */
enum class JobType { Processing, Streaming };

/**
 * @brief Converts a JobType to its single-character log representation ('P' or 'S').
 */
char jobTypeChar(JobType jobType);

/**
 * @brief Parses a dotted-decimal IP address string (e.g. "192.168.1.1") into its 32-bit form.
 */
uint32_t ipToLong(const std::string& ip);

/**
 * @brief Formats a 32-bit IP address as a dotted-decimal string (e.g. "192.168.1.1").
 */
std::string ipToString(uint32_t ip);

/**
 * @brief Represents a single network request with source/destination IPs, processing time, and job type.
 *
 * IPs are stored as 32-bit integers rather than strings: an address fits in
 * 4 bytes instead of a full std::string, and comparisons (e.g. firewall
 * range checks) become plain integer comparisons instead of string parsing.
 */
struct Request {
    uint32_t ipIn;   ///< Source IP address of the request
    uint32_t ipOut;  ///< Destination IP address for the response
    int time;           ///< Processing time in clock cycles
    JobType jobType;     ///< Job type: Processing or Streaming
    int arrivalCycle = -1; ///< Simulation cycle the request was generated, or -1 if unset
    int startCycle = -1;   ///< Simulation cycle a server began processing it, or -1 if unset

    /**
     * @brief Constructs a Request with the given parameters.
     * @param ipIn Source IP address
     * @param ipOut Destination IP address
     * @param time Processing time in clock cycles
     * @param jobType Job type
     */
    Request(uint32_t ipIn, uint32_t ipOut, int time, JobType jobType);

    /**
     * @brief Generates a random Request with random IPs and a random processing time.
     * @param minTime Minimum processing time
     * @param maxTime Maximum processing time
     * @return A randomly generated Request
     */
    static Request generateRandom(int minTime, int maxTime);

    /**
     * @brief Generates a random 32-bit IP address.
     * @return A random IP address
     */
    static uint32_t generateRandomIP();

    /**
     * @brief Seeds the shared random number generator used by all Request generation.
     * @param seed Seed value
     */
    static void seed(unsigned int seed);

    /**
     * @brief Draws a uniformly random integer in [min, max].
     */
    static int randomInt(int min, int max);

    /**
     * @brief Draws a uniformly random double in [0, 1).
     */
    static double randomDouble();
};

#endif
