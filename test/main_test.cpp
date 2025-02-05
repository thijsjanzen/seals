//
//  main.cpp
//  Seals
//
//  Created by Thijs Janzen on 09/12/2024.
//

#define CATCH_CONFIG_MAIN

#include <iostream>
#include "../parameters.h"
#include "../simulation.h"

#include "catch.h"

TEST_CASE("test initialization")
{
    parameters params;
    simulation sim(params);
    
    sim.initialize();
    
    CHECK(sim.mothers.size() == params.init_population_size);
    CHECK(sim.pups.size() == params.init_population_size);
    
    size_t id_counter = 0;
    auto test_indiv = individual(params.init_energy,
                                 life_stage::mother,
                                 ++id_counter);
    CHECK(test_indiv.current_location == location::colony);
    CHECK(test_indiv.milk == 0.0);
    CHECK(test_indiv.age == 0);
    CHECK(test_indiv.ID == id_counter);
    CHECK(test_indiv.time_since_pup_death == -1);
}

TEST_CASE("test birth")
{
    parameters params;
    simulation sim(params);
    
    sim.initialize();
    size_t id_counter = 0;
    auto test_indiv = individual(params.init_energy,
                                 life_stage::mother,
                                 ++id_counter);
    
    auto offspring = test_indiv.reproduce(++id_counter, params.init_offspring_energy);
    
    CHECK(offspring.age == 0);
    CHECK(offspring.mother_ID == test_indiv.ID);
    CHECK(offspring.ID == id_counter);
}

/*
int main(int argc, const char * argv[]) {
    std::cout << "This is running!" << std::endl;
    parameters params;
    params.read_from_config("config.ini");

    simulation sim(params);
    
    // here, we choose to run once, or do multiple replicates
    sim.run();
    
    return 0;
}
*/
