#ifndef REQUEST_H
#define REQUEST_H

#include <string>

/* Public request class fed to LB */
struct Request {
    std::string ipIn;
    std::string ipOut;
    int time; 
    char jobType;

    Request(std::string ipIn, std::string ipOut, int time, char jobType);
    static Request generateRandom(int minTime, int maxTime);
    static std::string generateRandomIP();

};

#endif
