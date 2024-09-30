

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

const static int GAME_LEN = 1000;

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
                        fitness += 176;
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
                        fitness += 10;
                        break;
                    case 2:
                        fitness += 5;
                        break;
                    case 3:
                        fitness += 0;
                        break;
                    case 4:
                        fitness += 100;
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
        return BotParameters::deserialize(buffer.str());
    } else {
        std::cerr << "Unable to open file for reading: " << filename << std::endl;
        return BotParameters();
    }
}

int main(int argc, char** argv) {
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

    std::random_device rd;
    std::mt19937 gen(rd());

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

        std::cout << "Generation " << generation << " Best Fitness: " << fitnesses[bestIndex] << std::endl;
    }

    std::vector<double> fitnesses(populationSize);
    for (int i = 0; i < populationSize; ++i) {
        fitnesses[i] = fitnessFunction(population[i], i);
    }

    // Save the best individual to a file
    auto bestIt = std::max_element(fitnesses.begin(), fitnesses.end());
    int bestIndex = std::distance(fitnesses.begin(), bestIt);
    write_params_to_file(population[bestIndex], outputFilename);

    return 0;
}

std::vector<BotParameters> initializePopulation(int populationSize, BotParameters base) {
    std::vector<BotParameters> population(populationSize, base);
    for (int i = 0; i < populationSize; i++) {
        mutate(population[i], 0.3);
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

BotParameters crossover(const BotParameters& parent1, const BotParameters& parent2) {
    BotParameters offspring;
    // Perform crossover between parent1 and parent2 to create offspring
    // For simplicity, we can use a single-point crossover
    offspring.hole_cost = (rand() % 2 == 0) ? parent1.hole_cost : parent2.hole_cost;
    offspring.overhang_cost = (rand() % 2 == 0) ? parent1.overhang_cost : parent2.overhang_cost;
    for (int i = 0; i < BotParameters::diff_count; ++i) {
        offspring.diff_cost[i] = (rand() % 2 == 0) ? parent1.diff_cost[i] : parent2.diff_cost[i];
    }
    offspring.larger_diff_cost = (rand() % 2 == 0) ? parent1.larger_diff_cost : parent2.larger_diff_cost;
    for (int i = 0; i < BotParameters::setup_count; ++i) {
        offspring.t_spin_setups_reward[i] = (rand() % 2 == 0) ? parent1.t_spin_setups_reward[i] : parent2.t_spin_setups_reward[i];
    }
    offspring.both_setup_reward = (rand() % 2 == 0) ? parent1.both_setup_reward : parent2.both_setup_reward;
    offspring.too_many_setups_cost = (rand() % 2 == 0) ? parent1.too_many_setups_cost : parent2.too_many_setups_cost;
    for (int i = 0; i < BOARD_HEIGHT; ++i) {
        offspring.max_height_reward[i] = (rand() % 2 == 0) ? parent1.max_height_reward[i] : parent2.max_height_reward[i];
        offspring.max_height_cost[i] = (rand() % 2 == 0) ? parent1.max_height_cost[i] : parent2.max_height_cost[i];
    }
    for (int i = 0; i < BOARD_WIDTH; ++i) {
        offspring.quad_hole_good_or_bad[i] = (rand() % 2 == 0) ? parent1.quad_hole_good_or_bad[i] : parent2.quad_hole_good_or_bad[i];
    }
    offspring.overhang_with_no_t_spin_cost = (rand() % 2 == 0) ? parent1.overhang_with_no_t_spin_cost : parent2.overhang_with_no_t_spin_cost;
    return offspring;
}

void mutate(BotParameters& individual, double mutationRate) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::normal_distribution<> mutation_mul(0.5, 1.5);
    std::normal_distribution<> mutation_add(-1.0, 1.0);

    if (dis(gen) < mutationRate) individual.hole_cost = (individual.hole_cost + mutation_add(gen)) * mutation_mul(gen);
    if (dis(gen) < mutationRate) individual.overhang_cost = (individual.overhang_cost + mutation_add(gen)) * mutation_mul(gen);
    for (int i = 0; i < BotParameters::diff_count; ++i) {
        if (dis(gen) < mutationRate) individual.diff_cost[i] = (individual.diff_cost[i] + mutation_add(gen)) * mutation_mul(gen);
    }
    if (dis(gen) < mutationRate) individual.larger_diff_cost = (individual.larger_diff_cost + mutation_add(gen)) * mutation_mul(gen);
    for (int i = 0; i < BotParameters::setup_count; ++i) {
        if (dis(gen) < mutationRate) individual.t_spin_setups_reward[i] = (individual.t_spin_setups_reward[i] + mutation_add(gen)) * mutation_mul(gen);
    }
    if (dis(gen) < mutationRate) individual.both_setup_reward = (individual.both_setup_reward + mutation_add(gen)) * mutation_mul(gen);
    if (dis(gen) < mutationRate) individual.too_many_setups_cost = (individual.too_many_setups_cost + mutation_add(gen)) * mutation_mul(gen);
    for (int i = 0; i < BOARD_HEIGHT; ++i) {
        if (dis(gen) < mutationRate) individual.max_height_reward[i] = (individual.max_height_reward[i] + mutation_add(gen)) * mutation_mul(gen);
        if (dis(gen) < mutationRate) individual.max_height_cost[i] = (individual.max_height_cost[i] + mutation_add(gen)) * mutation_mul(gen);
    }
    for (int i = 0; i < BOARD_WIDTH; ++i) {
        if (dis(gen) < mutationRate) individual.quad_hole_good_or_bad[i] = (individual.quad_hole_good_or_bad[i] + mutation_add(gen)) * mutation_mul(gen);
    }
    if (dis(gen) < mutationRate) individual.overhang_with_no_t_spin_cost = (individual.overhang_with_no_t_spin_cost + mutation_add(gen)) * mutation_mul(gen);
}
