// tune_heuristic.cpp
#include "game.hpp"
#include "player.hpp"
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

// Structure to hold a set of weights and its performance
struct WeightSet {
    HeuristicPlayer::Weights weights;
    double avgScore;
    int maxScore;
    int gamesPlayed;
    
    WeightSet(double emptyTiles, double monotonicity, double smoothness, double cornerPlacement)
        : avgScore(0), maxScore(0), gamesPlayed(0) {
        weights.emptyTiles = emptyTiles;
        weights.monotonicity = monotonicity;
        weights.smoothness = smoothness;
        weights.cornerPlacement = cornerPlacement;
    }
    
    // For sorting weight sets by average score
    bool operator<(const WeightSet& other) const {
        return avgScore < other.avgScore;
    }
    
    std::string toString() const {
        std::stringstream ss;
        ss << "Weights: [" 
           << std::fixed << std::setprecision(2) 
           << weights.emptyTiles << ", " 
           << weights.monotonicity << ", " 
           << weights.smoothness << ", " 
           << weights.cornerPlacement << "] "
           << "Avg Score: " << avgScore 
           << ", Max Score: " << maxScore 
           << ", Games: " << gamesPlayed;
        return ss.str();
    }
};

// Global counter for total games played
std::atomic<int> g_totalGamesPlayed(0);

// Evaluate a set of weights by playing multiple games
void evaluateWeights(WeightSet& weightSet, int numGames) {
    Game2048 game;
    int totalScore = 0;
    int maxScore = 0;
    
    for (int i = 0; i < numGames; ++i) {
        auto player = std::make_unique<HeuristicPlayer>(weightSet.weights);
        
        // Create a lambda that captures the player and calls its chooseAction method
        auto chooseActionFn = [&player](uint64_t state) {
            return player->chooseAction(state);
        };
        
        auto [score, state, moves] = game.playGame(chooseActionFn);
        
        totalScore += score;
        maxScore = std::max(maxScore, score);
        
        // Increment the global game counter
        g_totalGamesPlayed++;
    }
    
    weightSet.avgScore = static_cast<double>(totalScore) / numGames;
    weightSet.maxScore = maxScore;
    weightSet.gamesPlayed = numGames;
}

// Generate a random set of weights
WeightSet generateRandomWeights(std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    double emptyTiles = dist(rng);
    double monotonicity = dist(rng);
    double smoothness = dist(rng);
    double cornerPlacement = dist(rng);
    
    // Normalize weights to sum to 1.0
    double sum = emptyTiles + monotonicity + smoothness + cornerPlacement;
    emptyTiles /= sum;
    monotonicity /= sum;
    smoothness /= sum;
    cornerPlacement /= sum;
    
    return WeightSet(emptyTiles, monotonicity, smoothness, cornerPlacement);
}

// Generate a new weight set by mutating an existing one
WeightSet mutateWeights(const WeightSet& parent, std::mt19937& rng, double mutationRate) {
    std::normal_distribution<double> dist(0.0, mutationRate);
    
    double emptyTiles = std::max(0.0, std::min(1.0, parent.weights.emptyTiles + dist(rng)));
    double monotonicity = std::max(0.0, std::min(1.0, parent.weights.monotonicity + dist(rng)));
    double smoothness = std::max(0.0, std::min(1.0, parent.weights.smoothness + dist(rng)));
    double cornerPlacement = std::max(0.0, std::min(1.0, parent.weights.cornerPlacement + dist(rng)));
    
    // Normalize weights to sum to 1.0
    double sum = emptyTiles + monotonicity + smoothness + cornerPlacement;
    emptyTiles /= sum;
    monotonicity /= sum;
    smoothness /= sum;
    cornerPlacement /= sum;
    
    return WeightSet(emptyTiles, monotonicity, smoothness, cornerPlacement);
}

// Save the best weights to a file
void saveWeightsToFile(const std::vector<WeightSet>& weightSets, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    
    file << "# Heuristic Weights (emptyTiles, monotonicity, smoothness, cornerPlacement)\n";
    file << "# Format: emptyTiles,monotonicity,smoothness,cornerPlacement,avgScore,maxScore,gamesPlayed\n";
    
    for (const auto& weightSet : weightSets) {
        file << std::fixed << std::setprecision(4)
             << weightSet.weights.emptyTiles << ","
             << weightSet.weights.monotonicity << ","
             << weightSet.weights.smoothness << ","
             << weightSet.weights.cornerPlacement << ","
             << weightSet.avgScore << ","
             << weightSet.maxScore << ","
             << weightSet.gamesPlayed << "\n";
    }
    
    file.close();
    std::cout << "Weights saved to " << filename << std::endl;
}

// Load weights from a file
std::vector<WeightSet> loadWeightsFromFile(const std::string& filename) {
    std::vector<WeightSet> weightSets;
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
        std::vector<double> values;
        
        while (std::getline(ss, token, ',')) {
            values.push_back(std::stod(token));
        }
        
        if (values.size() >= 7) {
            WeightSet ws(values[0], values[1], values[2], values[3]);
            ws.avgScore = values[4];
            ws.maxScore = static_cast<int>(values[5]);
            ws.gamesPlayed = static_cast<int>(values[6]);
            weightSets.push_back(ws);
        }
    }
    
    std::cout << "Loaded " << weightSets.size() << " weight sets from " << filename << std::endl;
    return weightSets;
}

// Save the best weights to a separate file
void saveBestWeightsToFile(const WeightSet& bestWeights, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }
    
    // Write header
    file << "emptyTiles,monotonicity,smoothness,cornerPlacement,avgScore,maxScore,gamesPlayed" << std::endl;
    
    // Write best weights
    file << std::fixed << std::setprecision(6)
         << bestWeights.weights.emptyTiles << ","
         << bestWeights.weights.monotonicity << ","
         << bestWeights.weights.smoothness << ","
         << bestWeights.weights.cornerPlacement << ","
         << bestWeights.avgScore << ","
         << bestWeights.maxScore << ","
         << bestWeights.gamesPlayed << std::endl;
    
    std::cout << "Best weights saved to " << filename << std::endl;
}

// Evaluate multiple weight sets in parallel
void evaluateWeightsInParallel(std::vector<WeightSet>& weightSets, int numGames, int numThreads) {
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

int main(int argc, char* argv[]) {
    // Default parameters
    int populationSize = 20;
    int generations = 10;
    int gamesPerEvaluation = 30;  // Increased from 10 to 30
    double mutationRate = 0.1;
    double elitePercentage = 0.2;
    std::string outputFile = "heuristic_weights.csv";
    std::string bestWeightsFile = "best_weights.csv";
    bool continueFromFile = false;
    int numThreads = std::thread::hardware_concurrency();  // Default to available CPU cores
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--population" && i + 1 < argc) {
            populationSize = std::stoi(argv[++i]);
        } else if (arg == "--generations" && i + 1 < argc) {
            generations = std::stoi(argv[++i]);
        } else if (arg == "--games" && i + 1 < argc) {
            gamesPerEvaluation = std::stoi(argv[++i]);
        } else if (arg == "--threads" && i + 1 < argc) {
            numThreads = std::stoi(argv[++i]);
        } else if (arg == "--mutation" && i + 1 < argc) {
            mutationRate = std::stod(argv[++i]);
        } else if (arg == "--elite" && i + 1 < argc) {
            elitePercentage = std::stod(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "--best-output" && i + 1 < argc) {
            bestWeightsFile = argv[++i];
        } else if (arg == "--continue") {
            continueFromFile = true;
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  --population N    Set population size (default: 20)\n"
                      << "  --generations N   Set number of generations (default: 10)\n"
                      << "  --games N         Set games per evaluation (default: 30)\n"
                      << "  --threads N       Set number of parallel threads (default: " << numThreads << ")\n"
                      << "  --mutation X      Set mutation rate (default: 0.1)\n"
                      << "  --elite X         Set elite percentage (default: 0.2)\n"
                      << "  --output FILE     Set output file (default: heuristic_weights.csv)\n"
                      << "  --best-output FILE Set best weights output file (default: best_weights.csv)\n"
                      << "  --continue        Continue from existing weights file\n"
                      << "  --help            Show this help message\n";
            return 0;
        }
    }
    
    std::cout << "Heuristic Weight Tuning\n"
              << "----------------------\n"
              << "Population Size: " << populationSize << "\n"
              << "Generations: " << generations << "\n"
              << "Games per Evaluation: " << gamesPerEvaluation << "\n"
              << "Threads: " << numThreads << "\n"
              << "Mutation Rate: " << mutationRate << "\n"
              << "Elite Percentage: " << elitePercentage << "\n"
              << "Output File: " << outputFile << "\n"
              << "Best Weights Output File: " << bestWeightsFile << "\n"
              << "Continue from File: " << (continueFromFile ? "Yes" : "No") << "\n\n";
    
    // Random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    
    // Record start time
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Initialize population
    std::vector<WeightSet> population;
    
    if (continueFromFile) {
        population = loadWeightsFromFile(outputFile);
    }
    
    // If we don't have enough weights (or none were loaded), generate random ones
    while (population.size() < populationSize) {
        population.push_back(generateRandomWeights(rng));
    }
    
    // Add the default weights as a baseline
    population.push_back(WeightSet(0.2, 0.4, 0.1, 0.3));
    
    // Main evolutionary loop
    for (int gen = 0; gen < generations; ++gen) {
        std::cout << "Generation " << gen + 1 << "/" << generations << std::endl;
        
        // Evaluate all weight sets
        evaluateWeightsInParallel(population, gamesPerEvaluation, numThreads);
        
        // Sort by average score (best first)
        std::sort(population.begin(), population.end(), [](const WeightSet& a, const WeightSet& b) {
            return a.avgScore > b.avgScore;
        });
        
        // Print the best weights so far
        std::cout << "Best weights so far: " << population[0].toString() << std::endl;
        
        // Save intermediate results
        saveWeightsToFile(population, outputFile);
        
        // If this is the last generation, we're done
        if (gen == generations - 1) break;
        
        // Create the next generation
        std::vector<WeightSet> nextGeneration;
        
        // Keep the elite individuals
        int eliteCount = static_cast<int>(populationSize * elitePercentage);
        for (int i = 0; i < eliteCount && i < population.size(); ++i) {
            nextGeneration.push_back(population[i]);
        }
        
        // Generate new individuals through mutation
        while (nextGeneration.size() < populationSize) {
            // Select a parent using tournament selection
            std::uniform_int_distribution<size_t> dist(0, population.size() - 1);
            size_t idx1 = dist(rng);
            size_t idx2 = dist(rng);
            const WeightSet& parent = (population[idx1].avgScore > population[idx2].avgScore) 
                                     ? population[idx1] : population[idx2];
            
            // Create a mutated child
            WeightSet child = mutateWeights(parent, rng, mutationRate);
            child.avgScore = 0;
            child.maxScore = 0;
            child.gamesPlayed = 0;
            
            nextGeneration.push_back(child);
        }
        
        // Replace the population with the next generation
        population = nextGeneration;
    }
    
    // Final evaluation of the best weights
    std::cout << "\nFinal Evaluation\n";
    std::cout << "---------------\n";
    
    // Sort by average score (best first)
    std::sort(population.begin(), population.end(), [](const WeightSet& a, const WeightSet& b) {
        return a.avgScore > b.avgScore;
    });
    
    // Get the best weight set
    WeightSet& best = population[0];
    
    // Evaluate with more games for better accuracy
    std::cout << "Evaluating best weights with " << gamesPerEvaluation * 5 << " games...\n";
    
    // Create a vector with just the best weights for parallel evaluation
    std::vector<WeightSet> bestOnly = {best};
    best.gamesPlayed = 0; // Reset so it gets evaluated
    evaluateWeightsInParallel(bestOnly, gamesPerEvaluation * 5, numThreads);
    best = bestOnly[0];
    
    std::cout << "\nBest Weights Found:\n";
    std::cout << best.toString() << "\n\n";
    std::cout << "emptyTiles: " << best.weights.emptyTiles << "\n";
    std::cout << "monotonicity: " << best.weights.monotonicity << "\n";
    std::cout << "smoothness: " << best.weights.smoothness << "\n";
    std::cout << "cornerPlacement: " << best.weights.cornerPlacement << "\n";
    
    // Save all weights to the output file
    saveWeightsToFile(population, outputFile);
    
    // Save the best weights to a separate file for easy loading
    saveBestWeightsToFile(best, bestWeightsFile);
    
    // Calculate and display total time
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    int64_t hours = duration / 3600;
    int64_t minutes = (duration % 3600) / 60;
    int64_t seconds = duration % 60;
    
    std::cout << "\nPerformance Statistics:\n";
    std::cout << "---------------------\n";
    std::cout << "Total time: " << hours << "h " << minutes << "m " << seconds << "s\n";
    std::cout << "Total games played: " << g_totalGamesPlayed << "\n";
    std::cout << "Games per second: " << static_cast<double>(g_totalGamesPlayed) / duration << "\n";
    
    return 0;
} 