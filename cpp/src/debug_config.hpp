#ifndef DEBUG_CONFIG_HPP
#define DEBUG_CONFIG_HPP

struct DebugConfig {
    bool debug;
    bool printBoard;
    bool stepByStep;

    DebugConfig() :
        debug(false),
        printBoard(false),
        stepByStep(false)
    {}

    DebugConfig(bool debug, bool printBoard, bool stepByStep) :
        debug(debug),
        printBoard(printBoard),
        stepByStep(stepByStep)
    {}
};

#endif // DEBUG_CONFIG_HPP
