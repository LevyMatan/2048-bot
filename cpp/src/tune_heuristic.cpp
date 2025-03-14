// tune_heuristic.cpp
#include "game.hpp"
#include "players.hpp"
#include "evaluation.hpp"
#include "logger.hpp"
#include "arg_parser.hpp"
#include "score_types.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <unordered_map>
#include <set>



// Initialize logger
Logger2048::Logger &logger = Logger2048::Logger::getInstance();

// Structure to hold a set of evaluation weights and its performance
struct EvalWeightSet {
    Evaluation::EvalParams params;
    Score::GameScore avgScore;
    Score::GameScore maxScore;
    int gamesPlayed;
    std::set<std::string> activeComponents;

    EvalWeightSet()
        : avgScore(0), maxScore(0), gamesPlayed(0) {
    }

    // For sorting weight sets by average score
    bool operator<(const EvalWeightSet& other) const {
        return avgScore < other.avgScore;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "Weights: [";

        bool first = true;
        for (const auto& [name, weight] : params) {
            if (!first) ss << ", ";
            ss << name << ":" << weight;
            first = false;
        }

        ss << "] Avg Score: " << std::fixed << std::setprecision(1) << avgScore
           << ", Max Score: " << maxScore
           << ", Games: " << gamesPlayed
           << ", Active Components: " << activeComponents.size();
        return ss.str();
    }

    void printDetailedInfo() const {
        std::cout << "\nDetailed Evaluation Parameters:\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << std::left << std::setw(20) << "Component"
                  << std::setw(10) << "Weight"
                  << std::setw(10) << "% of Total" << "\n";
        std::cout << "------------------------------------------------------------\n";

        // Calculate total weight
        Evaluation::Weight totalWeight = 0;
        for (const auto& [name, weight] : params) {
            totalWeight += weight;
        }

        // Print each component's weight
        for (const auto& [name, weight] : params) {
            double percentage = static_cast<double>(weight) / totalWeight * 100.0;
            std::cout << std::left << std::setw(20) << name
                      << std::setw(10) << weight
                      << std::fixed << std::setprecision(1) << percentage << "%\n";
        }
        std::cout << "------------------------------------------------------------\n";
        std::cout << std::left << std::setw(20) << "Total"
                  << std::setw(10) << totalWeight
                  << "100.0%\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << "Average Score: " << std::fixed << std::setprecision(1) << avgScore << "\n";
        std::cout << "Maximum Score: " << maxScore << "\n";
        std::cout << "Games Played:  " << gamesPlayed << "\n";
        std::cout << "------------------------------------------------------------\n";
    }
};

// Global counter for total games played
std::atomic<int> g_totalGamesPlayed(0);

// Evaluate a set of weights by playing multiple games
void evaluateWeights(EvalWeightSet& weightSet, int numGames) {
    Game2048 game;
    Score::GameScore totalScore = 0;
    Score::GameScore maxScore = 0;

    for (int i = 0; i < numGames; ++i) {
        // Create a composite evaluator with the given parameters
        Evaluation::CompositeEvaluator evaluator(weightSet.params);
        auto evalFn = [evaluator](BoardState state) -> Score::BoardEval {
            return evaluator.evaluate(state);
        };

        auto player = std::make_unique<HeuristicPlayer>(weightSet.params);

        // Create a lambda that captures the player and calls its chooseAction method
        auto chooseActionFn = [&player](BoardState state) {
            return player->chooseAction(state);
        };

        auto [moveCount, finalState, score] = game.playGame(chooseActionFn, 0);

        totalScore += score;
        if (score > maxScore) {
            maxScore = score;
        }

        g_totalGamesPlayed++;
    }

    // Update the weight set with the results
    weightSet.avgScore = totalScore / numGames;
    weightSet.maxScore = maxScore;
    weightSet.gamesPlayed = numGames;
}

// Generate a random set of weights using available evaluation components
EvalWeightSet generateRandomWeights(std::mt19937& rng) {
    EvalWeightSet weightSet;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::uniform_int_distribution<int> boolDist(0, 1);

    // List of available components
    std::vector<std::string> components = {
        "emptyTiles", "monotonicity", "smoothness",
        "cornerValue", "mergeability", "patternMatching", "coreScore"
    };

    // Randomly decide which components to use (at least 2)
    std::vector<std::string> activeComponents;
    do {
        activeComponents.clear();
        for (const auto& comp : components) {
            if (boolDist(rng)) {
                activeComponents.push_back(comp);
            }
        }
    } while (activeComponents.size() < 2);

    // Generate random weights for active components
    std::vector<double> weights;
    double sum = 0.0;

    for (size_t i = 0; i < activeComponents.size(); ++i) {
        double w = dist(rng);
        weights.push_back(w);
        sum += w;
    }

    // Normalize weights to sum to 1000
    for (size_t i = 0; i < activeComponents.size(); ++i) {
        double normalizedWeight = (weights[i] / sum) * 1000.0;
        weightSet.params[activeComponents[i]] = normalizedWeight;
        weightSet.activeComponents.insert(activeComponents[i]);
    }

    return weightSet;
}

// Generate a new weight set by mutating an existing one
EvalWeightSet mutateWeights(const EvalWeightSet& parent, std::mt19937& rng, double mutationRate) {
    EvalWeightSet child;
    std::normal_distribution<double> dist(0.0, mutationRate * 1000.0); // Scale mutation rate to the 0-1000 range
    std::uniform_int_distribution<int> componentChangeDist(0, 3); // Probability of adding/removing a component

    // All available components
    std::vector<std::string> allComponents = {
        "emptyTiles", "monotonicity", "smoothness",
        "cornerValue", "mergeability", "patternMatching", "coreScore"
    };

    // Start with parent's active components
    child.activeComponents = parent.activeComponents;

    // Randomly add or remove components (with low probability)
    int componentChange = componentChangeDist(rng);
    if (componentChange == 0 && child.activeComponents.size() > 2) {
        // Remove a random component
        std::vector<std::string> components(child.activeComponents.begin(), child.activeComponents.end());
        std::uniform_int_distribution<size_t> indexDist(0, components.size() - 1);
        size_t indexToRemove = indexDist(rng);
        child.activeComponents.erase(components[indexToRemove]);
    } else if (componentChange == 1 && child.activeComponents.size() < allComponents.size()) {
        // Add a random component
        std::vector<std::string> unusedComponents;
        for (const auto& comp : allComponents) {
            if (child.activeComponents.find(comp) == child.activeComponents.end()) {
                unusedComponents.push_back(comp);
            }
        }
        if (!unusedComponents.empty()) {
            std::uniform_int_distribution<size_t> indexDist(0, unusedComponents.size() - 1);
            size_t indexToAdd = indexDist(rng);
            child.activeComponents.insert(unusedComponents[indexToAdd]);
        }
    }

    // Copy and mutate weights for active components
    double totalWeight = 0.0;

    for (const auto& component : child.activeComponents) {
        double weight;
        if (parent.params.find(component) != parent.params.end()) {
            // Mutate existing weight
            weight = std::max(0.0, parent.params.at(component) + dist(rng));
        } else {
            // Generate new weight for added component
            std::uniform_real_distribution<double> newWeightDist(50.0, 250.0);
            weight = newWeightDist(rng);
        }

        child.params[component] = weight;
        totalWeight += weight;
    }

    // Normalize weights to sum to 1000
    double scale = 1000.0 / totalWeight;
    double sum = 0;

    for (auto& [component, weight] : child.params) {
        weight = weight * scale;
        sum += weight;
    }

    // Adjust the last component to ensure the sum is exactly 1000
    if (!child.params.empty() && sum != 1000) {
        std::string lastComponent = *child.activeComponents.rbegin();
        child.params[lastComponent] += (1000 - sum);
    }

    return child;
}

// Save the weights to a file
void saveWeightsToFile(const std::vector<EvalWeightSet>& weightSets, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    file << "# Evaluation Weights\n";
    file << "# Format: component1:weight,component2:weight,...,avgScore,maxScore,gamesPlayed\n";

    for (const auto& weightSet : weightSets) {
        // Write all component weights
        bool first = true;
        for (const auto& comp : {"emptyTiles", "monotonicity", "smoothness",
                                "cornerValue", "mergeability", "patternMatching", "coreScore"}) {
            if (!first) file << ",";
            if (weightSet.params.find(comp) != weightSet.params.end()) {
                file << comp << ":" << weightSet.params.at(comp);
            } else {
                file << comp << ":0";
            }
            first = false;
        }

        // Write performance metrics
        file << "," << weightSet.avgScore
             << "," << weightSet.maxScore
             << "," << weightSet.gamesPlayed << "\n";
    }

    file.close();
    std::cout << "Weights saved to " << filename << std::endl;
}

// Load weights from a file
std::vector<EvalWeightSet> loadWeightsFromFile(const std::string& filename) {
    std::vector<EvalWeightSet> weightSets;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "No previous weights file found. Starting with default weights." << std::endl;
        return weightSets;
    }

    std::string line;
    // Skip header lines
    std::getline(file, line);
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 9) { // 6 components + 3 metrics
            EvalWeightSet ws;

            // Parse component weights
            for (int i = 0; i < 6; ++i) {
                std::string compToken = tokens[i];
                size_t colonPos = compToken.find(':');

                if (colonPos != std::string::npos) {
                    std::string component = compToken.substr(0, colonPos);
                    double weight = std::stod(compToken.substr(colonPos + 1));

                    if (weight > 0) {
                        ws.params[component] = weight;
                        ws.activeComponents.insert(component);
                    }
                }
            }

            // Parse performance metrics
            ws.avgScore = static_cast<Score::GameScore>(std::stod(tokens[7]));
            ws.maxScore = static_cast<Score::GameScore>(std::stoi(tokens[8]));
            ws.gamesPlayed = std::stoi(tokens[9]);

            weightSets.push_back(ws);
        }
    }

    std::cout << "Loaded " << weightSets.size() << " weight sets from " << filename << std::endl;
    return weightSets;
}

// Save the best weights to a separate file
void saveBestWeightsToFile(const EvalWeightSet& bestWeights, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    // Write header
    file << "# Best Evaluation Weights\n";

    // Write active components and their weights
    for (const auto& component : bestWeights.activeComponents) {
        file << component << ":" << bestWeights.params.at(component) << "\n";
    }

    // Write performance metrics
    file << "# Performance\n";
    file << "avgScore:" << bestWeights.avgScore << "\n";
    file << "maxScore:" << bestWeights.maxScore << "\n";
    file << "gamesPlayed:" << bestWeights.gamesPlayed << "\n";

    // Write ready-to-use code
    file << "\n# Ready-to-use code\n";
    file << "Evaluation::EvalParams params;\n";
    for (const auto& [name, weight] : bestWeights.params) {
        file << "params[\"" << name << "\"] = " << weight << ";\n";
    }

    // Also save in a format for getPresetParams function
    file << "\n# For getPresetParams function\n";
    file << "if (name == \"tuned\") {\n";
    for (const auto& [name, weight] : bestWeights.params) {
        file << "    params[\"" << name << "\"] = " << weight << ";\n";
    }
    file << "    return params;\n";
    file << "}\n";

    file.close();
    std::cout << "Best weights saved to " << filename << std::endl;

    // Save as JSON using the evaluation API
    if (Evaluation::saveParamsToJsonFile(bestWeights.params, "best_eval_weights.json")) {
        std::cout << "Best weights saved to best_eval_weights.json" << std::endl;
    } else {
        std::cerr << "Error: Could not save best weights to JSON file." << std::endl;
    }
}

// Analyze which components contribute most to good performance
void analyzeComponentContribution(const std::vector<EvalWeightSet>& population) {
    // Calculate the average score for the top 25% of the population
    std::vector<EvalWeightSet> sortedPopulation = population;
    std::sort(sortedPopulation.begin(), sortedPopulation.end(),
              [](const EvalWeightSet& a, const EvalWeightSet& b) {
                  return a.avgScore > b.avgScore;
              });

    size_t topCount = std::max(size_t(1), sortedPopulation.size() / 4);

    // Track component usage and average weights in top performers
    std::unordered_map<std::string, int> componentUsage;
    std::unordered_map<std::string, double> avgComponentWeight;
    std::unordered_map<std::string, double> avgComponentWeightNormalized;

    // All possible components
    std::vector<std::string> allComponents = {
        "emptyTiles", "monotonicity", "smoothness",
        "cornerValue", "mergeability", "patternMatching", "coreScore"
    };

    // Initialize usage counters
    for (const auto& comp : allComponents) {
        componentUsage[comp] = 0;
        avgComponentWeight[comp] = 0;
        avgComponentWeightNormalized[comp] = 0;
    }

    // Count usage in top performers
    for (size_t i = 0; i < topCount; ++i) {
        for (const auto& comp : allComponents) {
            if (sortedPopulation[i].activeComponents.find(comp) != sortedPopulation[i].activeComponents.end()) {
                componentUsage[comp]++;
                avgComponentWeight[comp] += sortedPopulation[i].params.at(comp);
            }
        }
    }

    // Calculate average weights
    for (const auto& comp : allComponents) {
        if (componentUsage[comp] > 0) {
            avgComponentWeight[comp] /= componentUsage[comp];
            avgComponentWeightNormalized[comp] = avgComponentWeight[comp] / 10.0; // Scale to percentage
        }
    }

    // Display results
    std::cout << "\nComponent Contribution Analysis (Top " << topCount << " performers):\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << std::left << std::setw(20) << "Component"
              << std::setw(12) << "Usage %"
              << std::setw(15) << "Avg Weight"
              << "Avg % of Total\n";
    std::cout << "------------------------------------------------------------\n";

    for (const auto& comp : allComponents) {
        double usagePercent = (componentUsage[comp] * 100.0) / topCount;

        std::cout << std::left << std::setw(20) << comp
                  << std::fixed << std::setprecision(1) << std::setw(12) << usagePercent
                  << std::setw(15) << avgComponentWeight[comp]
                  << avgComponentWeightNormalized[comp] << "%\n";
    }
    std::cout << "------------------------------------------------------------\n\n";
}

// Evaluate multiple weight sets in parallel
void evaluateWeightsInParallel(std::vector<EvalWeightSet>& weightSets, int numGames, int numThreads) {
    std::mutex outputMutex;
    std::vector<std::future<void>> futures;

    // Split the work among threads
    for (int t = 0; t < numThreads; ++t) {
        futures.push_back(std::async(std::launch::async, [&, t]() {
            // Each thread processes a subset of the weight sets
            for (size_t i = t; i < weightSets.size(); i += numThreads) {
                if (weightSets[i].gamesPlayed == 0) {
                    evaluateWeights(weightSets[i], numGames);

                    // Thread-safe output
                    {
                        std::lock_guard<std::mutex> lock(outputMutex);
                        std::cout << "Evaluated: " << weightSets[i].toString() << std::endl;
                    }
                }
            }
        }));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }
}

// Add new function
EvalWeightSet tournamentSelect(const std::vector<EvalWeightSet>& population, std::mt19937& rng, int tournamentSize = 3) {
    std::uniform_int_distribution<int> dist(0, static_cast<int>(population.size()) - 1);
    EvalWeightSet best = population[dist(rng)];

    for (int i = 1; i < tournamentSize; i++) {
        EvalWeightSet candidate = population[dist(rng)];
        if (candidate.avgScore > best.avgScore) {
            best = candidate;
        }
    }
    return best;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    TuneHeuristicParser argParser(argc, argv);
    TuneHeuristicParams params = argParser.getParams();

    logger.info(Logger2048::Group::Tuner, "Tuning Parameters:");
    logger.info(Logger2048::Group::Tuner, "  Population Size: " + std::to_string(params.populationSize));
    logger.info(Logger2048::Group::Tuner, "  Generations: " + std::to_string(params.generations));
    logger.info(Logger2048::Group::Tuner, "  Games per Evaluation: " + std::to_string(params.gamesPerEvaluation));
    logger.info(Logger2048::Group::Tuner, "  Mutation Rate: " + std::to_string(params.mutationRate));
    logger.info(Logger2048::Group::Tuner, "  Elite Percentage: " + std::to_string(params.elitePercentage));
    logger.info(Logger2048::Group::Tuner, "  Output File: " + params.outputFile);
    logger.info(Logger2048::Group::Tuner, "  Best Weights File: " + params.bestWeightsFile);
    logger.info(Logger2048::Group::Tuner, "  JSON Output File: " + params.jsonOutputFile);
    logger.info(Logger2048::Group::Tuner, "  Continue from File: " + std::string(params.continueFromFile ? "Yes" : "No"));
    logger.info(Logger2048::Group::Tuner, "  Threads: " + std::to_string(params.numThreads));
    logger.info(Logger2048::Group::Tuner, "  Verbosity: " + std::to_string(params.verbosity));
    logger.info(Logger2048::Group::Tuner, "");

    // Initialize random number generator
    std::random_device rd;
    std::mt19937 rng(rd());

    // Add before the main loop
    double initialMutationRate = params.mutationRate;

    // Add before main loop
    int generationsWithoutImprovement = 0;
    double bestScore = 0;
    const int MAX_GENERATIONS_WITHOUT_IMPROVEMENT = 5;

    double mutationRate = params.mutationRate; // Declare mutationRate here

    // Initialize population
    std::vector<EvalWeightSet> population;

    if (params.continueFromFile) {
        population = loadWeightsFromFile(params.outputFile);
    }

    // If not continuing or no weights were loaded, generate a new population
    if (population.empty()) {
        // Generate initial random weights
        for (int i = 0; i < params.populationSize; ++i) {
            population.push_back(generateRandomWeights(rng));
        }
    } else if (population.size() < static_cast<size_t>(params.populationSize)) {
        // If we loaded some but not enough, generate the rest
        int toGenerate = static_cast<int>(params.populationSize - population.size());
        for (int i = 0; i < toGenerate; ++i) {
            population.push_back(generateRandomWeights(rng));
        }
    }

    // Keep track of the best weights ever
    EvalWeightSet bestWeightSet;
    bestWeightSet.avgScore = 0;

    // Run the evolutionary algorithm
    for (int gen = 0; gen < params.generations; ++gen) {
        std::cout << "\n===== Generation " << gen + 1 << " =====\n";

        // In the generation loop
        mutationRate = initialMutationRate * (1.0 - static_cast<double>(gen) / params.generations);

        // Evaluate all weight sets
        evaluateWeightsInParallel(population, params.gamesPerEvaluation, params.numThreads);

        // Sort by average score (descending)
        std::sort(population.begin(), population.end(), [](const EvalWeightSet& a, const EvalWeightSet& b) {
            return a.avgScore > b.avgScore;
        });

        // Update best weights
        if (population[0].avgScore > bestWeightSet.avgScore) {
            bestWeightSet = population[0];
            std::cout << "\n*** NEW BEST SCORE FOUND ***\n";
            population[0].printDetailedInfo();
        }

        // Print the best weights for this generation
        std::cout << "\nBest weights this generation: " << population[0].toString() << "\n";
        std::cout << "Best weights overall: " << bestWeightSet.toString() << "\n";
        std::cout << "Total games played: " << g_totalGamesPlayed << "\n";

        // Analyze component contribution for this generation
        analyzeComponentContribution(population);

        // Save all weights
        saveWeightsToFile(population, params.outputFile);

        // Save the best weights
        saveBestWeightsToFile(bestWeightSet, params.bestWeightsFile);

        // Save the best weights to JSON file
        if (Evaluation::saveParamsToJsonFile(bestWeightSet.params, params.jsonOutputFile)) {
            std::cout << "Best weights saved to " << params.jsonOutputFile << std::endl;
        } else {
            std::cerr << "Error: Could not save best weights to JSON file." << std::endl;
        }

        // Add in the generation loop, e.g., every 5 generations
        if (gen % 5 == 0) {
            std::string checkpointFile = "checkpoint_gen_" + std::to_string(gen) + ".csv";
            saveWeightsToFile(population, checkpointFile);
        }

        // In the generation loop after evaluation
        double currentBestScore = static_cast<double>(population[0].avgScore);
        if (currentBestScore > bestScore) {
            bestScore = currentBestScore;
            generationsWithoutImprovement = 0;
        } else {
            generationsWithoutImprovement++;
            if (generationsWithoutImprovement >= MAX_GENERATIONS_WITHOUT_IMPROVEMENT) {
                std::cout << "Stopping early due to no improvement in "
                          << MAX_GENERATIONS_WITHOUT_IMPROVEMENT << " generations\n";
                break;
            }
        }

        // If this is the last generation, we're done
        if (gen == params.generations - 1) {
            break;
        }

        // Create the next generation
        std::vector<EvalWeightSet> nextGeneration;

        // Keep the elites
        int eliteCount = static_cast<int>(params.elitePercentage * params.populationSize);
        for (int i = 0; i < eliteCount; ++i) {
            nextGeneration.push_back(population[i]);
        }

        // Create the rest through mutation and crossover
        std::uniform_int_distribution<int> parentDist(0, std::min(params.populationSize - 1, static_cast<int>(params.populationSize * 0.5)));

        while (nextGeneration.size() < static_cast<size_t>(params.populationSize)) {
            EvalWeightSet parent = tournamentSelect(population, rng);
            EvalWeightSet child = mutateWeights(parent, rng, mutationRate);
            nextGeneration.push_back(child);
        }

        // Replace the current generation with the next
        population = nextGeneration;
    }

    std::cout << "\n===== Tuning Complete =====\n";
    std::cout << "Best weights found: " << bestWeightSet.toString() << "\n";
    std::cout << "Total games played: " << g_totalGamesPlayed << "\n";

    // Print detailed information about the best weights
    std::cout << "\n===== FINAL RESULTS =====\n";
    bestWeightSet.printDetailedInfo();

    // Print a ready-to-use parameter configuration
    std::cout << "\nReady-to-use parameter configuration:\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << "Evaluation::EvalParams params;\n";
    for (const auto& [name, weight] : bestWeightSet.params) {
        std::cout << "params[\"" << name << "\"] = " << weight << ";\n";
    }
    std::cout << "------------------------------------------------------------\n";

    return 0;
}