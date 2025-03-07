#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config {
    std::string evalName = "basic";
    int depth = 3;
    double timeLimit = 0.1;
    bool adaptiveDepth = true;
    bool debug = false;
    bool stepByStep = false;
    bool printBoard = false;
};

#endif
