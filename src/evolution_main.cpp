

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

#include "Stacker.h"
#include "StackerBot.h"

using namespace Stacker;

const static int GAME_LEN = 10000;

double fitnessFunction(const BotParameters& params, int seed) {
    double fitness = 0;
    StackerBot bot(params, seed);

    for (int i = 0; i < GAME_LEN; i++) {
        auto move = bot.suggest_move();
        do_move(move, bot.get_game());
        bot.get_game().tick();
        auto clear_info = bot.get_game().get_last_clear();
        if (clear_info.clear_count > 0) {
            if (clear_info.is_spin) {
                switch (clear_info.clear_count) {
                    case 1:
                        fitness += 75;
                        break;
                    case 2:
                        fitness += 175;
                        break;
                    case 3:
                        fitness += 500;
                        break;
                    case 4:
                        fitness += 800;
                        break;
                    default:
                        break;
                }
            } else {
                switch (clear_info.clear_count) {
                    case 1:
                        fitness += 0;
                        break;
                    case 2:
                        fitness += 0;
                        break;
                    case 3:
                        fitness += 0;
                        break;
                    case 4:
                        fitness += 150;
                        break;
                    default:
                        break;
                }
            }
        }
        bot.get_game().empty_last_clear();
        if (bot.get_game().is_dead()) {
            fitness /= 10;
            break;
        }
    }
    return fitness;
}

// Function prototype
std::vector<BotParameters> initializePopulation(int populationSize, BotParameters base);
BotParameters selectParent(const std::vector<BotParameters>& population, const std::vector<double>& fitnesses);
BotParameters crossover(const BotParameters& parent1, const BotParameters& parent2);
void mutate(BotParameters& individual, double mutationRate);

void write_params_to_file(const BotParameters& params, const std::string& filename) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << params.serialize();
    } else {
        std::cerr << "Unable to open file for writing: " << filename << std::endl;
    }
}

BotParameters load_params_from_file(const std::string& filename) {
    std::ifstream inFile(filename);
    if (inFile.is_open()) {
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        BotParameters res;
        res.deserialize(buffer.str());
        return res;
    } else {
        std::cerr << "Unable to open file for reading: " << filename << std::endl;
        return BotParameters();
    }
}

std::vector<BotParameters> initializePopulationRandom(int populationSize) {
    std::vector<BotParameters> population(populationSize);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    for (int i = 0; i < populationSize; i++) {
        for (int j = 0; j < population[i].TOTAL_SIZE; j++) {
            population[i].data[j] = dis(gen);
        }
    }
    // Initialize each BotParameters object as needed
    return population;
}

int main(int argc, char** argv) {
    if (std::string(argv[1]) == std::string("test_one_fitness")) {
        auto params = load_params_from_file("evolution/god.txt");

        auto start = std::chrono::high_resolution_clock::now();
        double fitness = fitnessFunction(params, 0);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        std::cout << "Fitness: " << fitness << std::endl;
        std::cout << "Duration: " << duration.count() << " seconds" << std::endl;
        return 0;
    }
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <populationSize> <generations> <mutationRate>" << std::endl;
        return 1;
    }

    const int populationSize = std::stoi(argv[1]);
    const int generations = std::stoi(argv[2]);
    const double mutationRate = std::stod(argv[3]);
    const std::string outputFilename = "evolution/best_individual.txt";

    auto god = load_params_from_file("evolution/god.txt");
    // BotParameters god;
    write_params_to_file(god, "evolution/lucifer.txt");
    BotParameters loser;
    write_params_to_file(loser, "loser.txt");

    // Initialize population
    std::vector<BotParameters> population = initializePopulation(populationSize, god);
    if (argc >= 5  && std::string(argv[4]) == std::string("--random")) {
        population = initializePopulationRandom(populationSize);
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    int the_goat_score = -1;
    BotParameters the_goat;

    for (int generation = 0; generation < generations; ++generation) {
        // Evaluate fitness
        std::vector<std::thread*> threads(populationSize);
        std::vector<double> fitnesses(populationSize);
        for (int i = 0; i < populationSize; ++i) {
            threads[i] = new std::thread([&fitnesses, &population, i]() {
                fitnesses[i] = fitnessFunction(population[i], i);
            });
        }

        for (int i = 0; i < populationSize; ++i) {
            threads[i]->join();
            delete threads[i];
        }

        // Create new generation
        std::vector<BotParameters> newPopulation;
        for (int i = 0; i < populationSize; ++i) {
            BotParameters parent1 = selectParent(population, fitnesses);
            BotParameters parent2 = selectParent(population, fitnesses);
            BotParameters offspring = crossover(parent1, parent2);
            mutate(offspring, mutationRate);
            newPopulation.push_back(offspring);
        }

        population = newPopulation;

        // Output the best individual of the current generation
        auto bestIt = std::max_element(fitnesses.begin(), fitnesses.end());
        int bestIndex = std::distance(fitnesses.begin(), bestIt);
        write_params_to_file(population[bestIndex], "evolution/midway/generation_" + std::to_string(generation) + "_" + std::to_string(fitnesses[bestIndex]));
        if (fitnesses[bestIndex] > the_goat_score) {
            the_goat = population[bestIndex];
        }

        std::cout << "Generation " << generation << " Best Fitness: " << fitnesses[bestIndex] << std::endl;
    }

    // Save the best individual to a file
    write_params_to_file(the_goat, "evolution/the_goat.txt");
    return 0;
}

std::vector<BotParameters> initializePopulation(int populationSize, BotParameters base) {
    std::vector<BotParameters> population(populationSize, base);
    for (int i = 0; i < populationSize; i++) {
        mutate(population[i], 0);
    }
    // Initialize each BotParameters object as needed
    return population;
}



BotParameters selectParent(const std::vector<BotParameters>& population, const std::vector<double>& fitnesses) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<double> fitnesses_cubed(fitnesses.size());
    std::transform(fitnesses.begin(), fitnesses.end(), fitnesses_cubed.begin(), [](double fitness) {
        return fitness * fitness * fitness;
    });
    std::discrete_distribution<> dist(fitnesses_cubed.begin(), fitnesses_cubed.end());
    return population[dist(gen)];
}

void normalize(BotParameters& params) {
    double factor = 0;
    for (int i = 0; i < BotParameters::TOTAL_SIZE; i++) {
        factor += params.data[i] * params.data[i];
    }
    factor = std::sqrt(factor);
    if (factor == 0) {
        return;
    }
    factor = 1 / factor;
    factor *= 1000;  // make it not tiny
    for (int i = 0; i < BotParameters::TOTAL_SIZE; i++) {
        params.data[i] *= factor;
    }
}

BotParameters crossover(const BotParameters& parent1, const BotParameters& parent2) {
    BotParameters offspring;
    BotParameters norm1 = parent1;
    normalize(norm1);
    BotParameters norm2 = parent2;
    normalize(norm2);

    auto distro = std::uniform_real_distribution<>(0, 1);
    auto rng = std::mt19937(std::random_device{}());

    for (int i = 0; i < BotParameters::TOTAL_SIZE; i++) {
        offspring.data[i] = (distro(rng) == 0) ? norm1.data[i] : norm2.data[i];
    }
    return offspring;
}

void mutate(BotParameters& individual, double mutationRate) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::normal_distribution<> mutation_mul(0.5, 1.5);
    std::normal_distribution<> mutation_add(-1.0, 1.0);

    for (int i = 0; i < BotParameters::TOTAL_SIZE; i++) {
        if (dis(gen) < mutationRate) individual.data[i] = (individual.data[i] + mutation_add(gen)) * mutation_mul(gen);
    }
}
